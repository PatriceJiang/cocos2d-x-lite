
#include "jsb_jni_utils.h"

#include <cassert>

namespace jni_utils
{

    JniType JniType::from(JniTypeEnum e)
    {
        JniType ret;
        switch (e) {
            case JniTypeEnum::Void_V:
            case JniTypeEnum::Boolean_Z:
            case JniTypeEnum::Char_C:
            case JniTypeEnum::Byte_B:
            case JniTypeEnum::Short_S:
            case JniTypeEnum::Int_I:
            case JniTypeEnum::Long_J:
            case JniTypeEnum::Float_F:
            case JniTypeEnum::Double_D:
                ret.type = e;
                return ret;
            default:
                break;
        }
        return ret;
    }


    std::string JniType::toString() const {
        std::stringstream ss;
        auto d = this->dim;
        while (d > 0)
        {
            ss << "[";
            d--;
        }
        switch (type)
        {
        case jni_utils::JniTypeEnum::None:
            ss << "";
            break;
        case jni_utils::JniTypeEnum::Void_V:
            ss << "V";
            break;
        case jni_utils::JniTypeEnum::Boolean_Z:
            ss << "Z";
            break;
        case jni_utils::JniTypeEnum::Char_C:
            ss << "C";
            break;
        case jni_utils::JniTypeEnum::Byte_B:
            ss << "B";
            break;
        case jni_utils::JniTypeEnum::Short_S:
            ss << "S";
            break;
        case jni_utils::JniTypeEnum::Int_I:
            ss << "I";
            break;
        case jni_utils::JniTypeEnum::Long_J:
            ss << "J";
            break;
        case jni_utils::JniTypeEnum::Float_F:
            ss << "F";
            break;
        case jni_utils::JniTypeEnum::Double_D:
            ss << "D";
            break;
        case jni_utils::JniTypeEnum::Object_L:
            ss << "L" << klassName << ";";
            break;
        default:
            assert(false);
            break;
        }
        return ss.str();
    }

    bool parseSigType(const char *data, size_t max_length, size_t *len, JniType *type) {
        size_t i = 0;
        const char f = data[i];
        *len += 1;
        switch(f) {
            case 'V':
                type->type = JniTypeEnum::Void_V;
                return true;
            case 'Z':
                type->type = JniTypeEnum::Boolean_Z;
                return true;
            case 'C':
                type->type = JniTypeEnum::Char_C;
                return true;
            case 'S':
                type->type = JniTypeEnum::Short_S;
                return true;
            case 'B':
                type->type = JniTypeEnum::Byte_B;
                return true;
            case 'I':
                type->type = JniTypeEnum::Int_I;
                return true;
            case 'J':
                type->type = JniTypeEnum::Long_J;
                return true;
            case 'F' :
                type->type = JniTypeEnum::Float_F;
                return true;
            case 'D' :
                type->type = JniTypeEnum::Double_D;
                return true;
//            case ';':
//                return true;
            default: ;
        }

        if(f == '[') {
            type->dim += 1;
            return parseSigType(data + 1, max_length -1, len, type);
        }else if(f == 'L') {
            i = 1;
            while(i < max_length && data[i] != ';') {
                i++;
            }
            if(i == max_length) {
                return false;
            }
            type->klassName = std::string(data + 1, i - 1);
            type->type= JniTypeEnum::Object_L;
            *len += i;
            return true;
        }
        return false;
    }

    std::vector<jni_utils::JniType> exactArgsFromSignature(const std::string &signature, bool& success)
    {
        int i = 0;
        const int e = signature.length();
        if (signature[i] != '(') {
            success = false;
            return {};
        }
        auto qtR = signature.find_last_of(')');
        if (qtR == std::string::npos) {
            success = false;
            return {};
        }
        const char* data = signature.data() + 1;
        size_t max_length = qtR - 1;
        size_t delta = 0;
        jni_utils::JniType localType;
        bool ok = false;
        std::vector<jni_utils::JniType> argTypeList;
        do {
            ok = jni_utils::parseSigType(data, max_length, &delta, &localType);
            if (ok) {
                argTypeList.push_back(std::move(localType));
                data += delta;
                max_length -= delta;
                delta = 0;
            }
        } while (ok);

        success = true;

        return argTypeList;
    }

} // namespace jni_utils
