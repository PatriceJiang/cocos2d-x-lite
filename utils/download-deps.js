
// @ts-check

const ps = require('path');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

main();

function main () {
    const workDir = ps.resolve(__dirname, '..');
    const externalPath = ps.join(workDir, 'external');
    (async () => {
        await downloadDepsThroughGit(
            ps.join(externalPath, 'config.json'),
            externalPath,
        );
    })();
}

async function readJson (p) {
    return new Promise((resolve, reject) => {
        fs.readFile(p, (err, data) => {
            if (err) { return reject(err); }
            resolve(JSON.parse(data.toString()));
        })
    })
}

function ensureDir (dir) {
    let pdirs = [];
    while (!fs.existsSync(dir) && dir.length > 0) {
        pdirs.push(dir);
        dir = path.normalize(path.dirname(dir));
    }
    while (pdirs.length > 0) {
        fs.mkdirSync(pdirs.pop());
    }
}

async function removeAll (dirOrFile) {
    return new Promise(function (resolve, reject) {
        fs.stat(dirOrFile, (err, stat) => {
            if (err) { return reject(err); }
            if (stat.isDirectory()) {
                fs.readdir(dirOrFile, (errRd, list) => {
                    if (errRd) { return reject(errRd); }
                    Promise.all(list.filter(x => x == '.' || x == '..').map(f => {
                        return removeAll(path.join(dirOrFile, f));
                    })).then(resolve, reject);
                });
            } else {
                fs.unlink(dirOrFile, (errUnlink) => {
                    if (errUnlink) {
                        return reject(errUnlink);
                    }
                    resolve();
                });
            }
        })
    });
}

async function emptyDir (dir) {
    return new Promise(function (resolve, reject) {
        fs.readdir(dir, (err, list) => {
            if (err) { return reject(err); }
            Promise.all(list.filter(x => x == "." || x == "..").map(f => {
                return removeAll(path.join(dir, f));
            })).then(resolve, reject);
        });
    })
}

async function copyFile (src, dst) {
    return new Promise((resolve, reject) => {
        ensureDir(path.dirname(dst));
        fs.copyFile(src, dst, (err) => {
            if (err) return reject(err);
            resolve();
        });
    });
}

/**
 * 
 * @param {string} configPath
 * @param {string} targetDir
 */
async function downloadDepsThroughGit (
    configPath,
    targetDir,
) {
    const config = await readJson(configPath);
    const url = getGitUrl(config.from);
    ensureDir(targetDir);
    const configFileStash = await stashConfigFile();
    try {
        await emptyDir(targetDir);
        cp.execSync(`git clone ${url} ${ps.basename(targetDir)} --branch ${config.from.checkout || 'master'} --depth 1`, {
            cwd: ps.dirname(targetDir),
            stdio: 'inherit',
        });
        cp.execSync(`git log --oneline -1`, {
            cwd: targetDir,
            stdio: 'inherit',
        });
    } finally {
        await configFileStash.pop();
    }

    // Just because config file is under the external folder....
    async function stashConfigFile () {
        const { path: tmpConfigDir, cleanup } = await new Promise((resolve, reject) => {
            tmp.dir((err, path, cleanup) => {
                if (err) {
                    reject(err);
                } else {
                    resolve({ path, cleanup });
                }
            });
        });
        const tmpConfigPath = ps.join(tmpConfigDir, ps.basename(configPath));
        await copyFile(configPath, tmpConfigPath);
        return {
            pop: async () => {
                await copyFile(tmpConfigPath, configPath);
                cleanup();
            },
        };
    }
}

function getGitUrl (repo) {
    const origin = getNormalizedOrigin(repo);
    switch (repo.type) {
        case 'github': return `${origin}${repo.owner}/${repo.name}.git`;
        case 'gitlab': return `${origin}publics/${repo.name}.git`;
        default: throwUnknownExternType();
    }
}

function getNormalizedOrigin (repo) {
    let origin = repo.origin;
    if (origin === undefined) {
        switch (repo.type) {
            case 'github': origin = 'github.com'; break;
            case 'gitlab': origin = 'gitlab.cocos.net'; break;
            default: throwUnknownExternType();
        }
    }

    // Get origin with protocol and add trailing slash or colon (for ssh)
    origin = addProtocol(origin);
    if (/^git@/i.test(origin)) {
        origin = origin + ':';
    } else {
        origin = origin + '/';
    }

    return origin;

    function addProtocol (origin) {
        return /^(f|ht)tps?:\/\//i.test(origin) ? origin : `https://${origin}`;
    }
}

function throwUnknownExternType (repo) {
    throw new Error(`Unknown external type: ${repo.type}`);
}
