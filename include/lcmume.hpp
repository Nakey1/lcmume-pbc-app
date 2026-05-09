#include <cstring>
#include <cassert>
#include <utility>
#include <cstring>
#include <pbc/pbc.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <openssl/sha.h>
#include <memory>

struct PairingGuard {
    pairing_t p;
    std::string ps;

    bool initialized_ = false;

    explicit PairingGuard(const std::string& s) :ps(s){
        ps=s;
        if (pairing_init_set_str(p, s.c_str()) == 0) initialized_ = true;
    }

    // 禁用拷贝（Pairing 资源是唯一的）
    PairingGuard(const PairingGuard&) = delete;
    PairingGuard& operator=(const PairingGuard&) = delete;

    ~PairingGuard() {
        if (initialized_) {
            pairing_clear(p);
            std::cout << "Pairing 资源已安全回收。" << std::endl;
        }
    }
};

class Element {
private:
    element_t e;
    bool initialized_ = false;

public:
    Element() : initialized_(false) {
        std::memset(e, 0, sizeof(element_t));
    }

    Element& init_G1(PairingGuard& guard) {
        if (initialized_) element_clear(e);
        element_init_G1(e, guard.p);
        initialized_ = true;
        return *this;
    }


    Element& init_Zr(PairingGuard& guard) {
        if (initialized_) element_clear(e);
        element_init_Zr(e, guard.p);
        initialized_ = true;
        return *this;
    }

 
    ~Element() noexcept {
        if (initialized_) {
            element_clear(e);
            initialized_ = false;
        }
    }

    operator element_s*() { 
        return e; 
    }

    element_s* get() { 
        return e; 
    }

    bool is_initialized() const { return initialized_; }

    // 禁用拷贝（RAII 唯一所有权语义）
    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;

   // 移动构造
    Element(Element&& other) noexcept : initialized_(other.initialized_)
    {
        if (initialized_) {
            std::memcpy(e, other.e, sizeof(element_t));
            std::memset(other.e, 0, sizeof(element_t));
            other.initialized_ = false; // 剥夺原对象所有权
            

        }else {
            std::memset(e, 0, sizeof(element_t));
        }
    }

// 移动赋值
    Element& operator=(Element&& other) noexcept {
        if (this != &other) {
            // 先清理自身资源
            if (initialized_) {
                element_clear(e);
            }

            initialized_=other.initialized_;

            if(initialized_){
                std::memcpy(e, other.e, sizeof(element_t));
                std::memset(other.e, 0, sizeof(element_t));
                other.initialized_ = false;

            }else {
                std::memset(e, 0, sizeof(element_t));
            }
            
            
        }   
        return *this;
    }

    // 显式克隆函数 (代替拷贝构造函数，避免隐式性能损耗)
    Element clone() const {
        Element replica;
        if (initialized_) {
            element_init_same_as(replica.e, const_cast<element_s*>(e));
            element_set(replica.e, const_cast<element_s*>(e));
            replica.initialized_ = true;
        }
        return replica; // 触发移动语义返回
    }

    void print(const std::string& label = "") const {
        if (!initialized_) {
            std::cout << label << ": [未初始化]" << std::endl;
            return;
        }
        if (!label.empty()) std::cout << label << ": ";
        element_printf("%B\n", e);
    }

    std::string to_string() const {
        if (!initialized_) {
            return ""; 
        }
        char buf[2048];
        element_snprintf(buf, sizeof(buf), "%B", e);
        return std::string(buf);
    }

    bool from_string(const std::string& str, int base = 10) {
        if (!initialized_) {
            // 必须先调用 init_G1 或 init_Zr 分配内存后才能写入数据
            return false; 
        }
        // element_set_str 返回实际写入的字符数，若为 0 则说明解析失败
        if (element_set_str(e, str.c_str(), base) == 0) {
            return false;
        }
        return true;
    }
};



// 基础参数：客户端和服务器共有
struct CryptoContext {
    // --- 算法常量映射 ---
    static constexpr int H2_LEN = 64;   // C3 长度
    static constexpr int H3_LEN = 32;   // C4 长度 (完整性校验)
    static constexpr int MSG_LEN = 32;  // 明文最大长度

    // --- 运行环境 ---
    std::unique_ptr<PairingGuard> guard_;
    Element G;
    Element P_pub;

    // 构造函数可以处理初始化逻辑
    explicit CryptoContext(const std::string& param_str) {
        guard_ = std::make_unique<PairingGuard>(param_str);


        if (!guard_->initialized_) {
        throw std::runtime_error("pairing init failed");
    }
    
        G.init_G1(*guard_);
        P_pub.init_G1(*guard_);
        std::cout<< "CryptoContext: 环境初始化成功。" << std::endl;
    }
    
    // 禁止拷贝，防止 pairing 被多次清理导致崩溃
    CryptoContext(const CryptoContext&) = delete;
    CryptoContext& operator=(const CryptoContext&) = delete;

    CryptoContext(CryptoContext&& other) noexcept = default;
    CryptoContext& operator=(CryptoContext&& other) noexcept = default;

    ~CryptoContext()  {
        std::cout << "CryptoContext: 环境资源已安全回收。" << std::endl;
    }
};

// 服务器专用参数：继承基础参数
struct ServerContext : public CryptoContext {
    Element msk;

    ServerContext(const std::string& param_str) : CryptoContext(param_str) {
        msk.init_Zr(*guard_); // 服务器特有的主私钥
    }

    std::string get_param_str() const {
        return guard_->ps;
    }

    
    
};

struct UserKeys {
    std::string id;
    Element x;  // 私钥1 (Zr)
    Element d;  // 私钥2 (Zr)
    Element X;  // 公钥1 (G1)
    Element R;  // 公钥2 (G1)
    


    UserKeys() = default;
    UserKeys(UserKeys&&) noexcept = default;
    UserKeys& operator=(UserKeys&&) noexcept = default;

    // 禁用拷贝，确保密钥对象的唯一性
    UserKeys(const UserKeys&) = delete;
    UserKeys& operator=(const UserKeys&) = delete;
};

// 服务器端存储的用户
struct ServerUserKeys {
    std::string id;
    Element d;  // Zr
    Element R;  // G1
    Element X;  // G1
};

struct PeerUserKeys {
    std::string id;
    Element X;  // 对方的公钥1 (G1)
    Element R;  // 对方的公钥2 (G1)

    PeerUserKeys() = default;
    PeerUserKeys(PeerUserKeys&&) noexcept = default;
    PeerUserKeys& operator=(PeerUserKeys&&) noexcept = default;
    
    PeerUserKeys(const PeerUserKeys&) = delete;
    PeerUserKeys& operator=(const PeerUserKeys&) = delete;
};

struct Ciphertext {
    Element C1; // 对应群 G1 中的点
    Element C2; // 对应群 G1 中的点
    
 
    std::array<unsigned char, CryptoContext::H2_LEN> C3;
    std::array<unsigned char, CryptoContext::H3_LEN> C4;

    std::vector<Element> poly_coeffsV;
    std::vector<Element> poly_coeffsZ;

    Ciphertext() = default;

    Ciphertext(const Ciphertext&) = delete;
    Ciphertext& operator=(const Ciphertext&) = delete;

    Ciphertext(Ciphertext&&) noexcept = default;
    Ciphertext& operator=(Ciphertext&&) noexcept = default;


    void clear() {
        poly_coeffsV.clear();
        poly_coeffsZ.clear();
        C3.fill(0);
        C4.fill(0);
    }
};

inline void H_point_to_Zr(CryptoContext& ctx, Element& out_Zr, Element& point) {
    if (!out_Zr.is_initialized()) out_Zr.init_Zr(*ctx.guard_);

    int len = element_length_in_bytes(point);
    std::vector<unsigned char> buffer(len);
    element_to_bytes(buffer.data(), point);

    element_from_hash(out_Zr, buffer.data(), len);
}

inline void H1_map(CryptoContext& ctx, Element& out_Zr, const std::string& id, Element& R) {
    
    int R_len = element_length_in_bytes(R);

    std::vector<unsigned char> buffer(id.length() + R_len);
    memcpy(buffer.data(), id.c_str(), id.length());
    element_to_bytes(buffer.data() + id.length(), R);

    // 3. 初始化输出
    if (!out_Zr.is_initialized()) {
        out_Zr.init_Zr(*ctx.guard_);
    }

    element_from_hash(out_Zr, buffer.data(), buffer.size());
}



inline void H2_map(std::array<unsigned char, CryptoContext::H2_LEN>& out_buf, 
            Element& g1, Element& g2, Element& d1, Element& d2) {
    
    int single_len = element_length_in_bytes(g1);
    std::vector<unsigned char> buffer(single_len * 4);
    
    element_to_bytes(buffer.data(), g1);
    element_to_bytes(buffer.data() + single_len, g2);
    element_to_bytes(buffer.data() + single_len * 2, d1);
    element_to_bytes(buffer.data() + single_len * 3, d2);

    SHA512(buffer.data(), buffer.size(), out_buf.data());
}

inline void H3_map(std::array<unsigned char, CryptoContext::H3_LEN>& out_buf, 
            Element& c1, Element& c2, 
            std::array<unsigned char, CryptoContext::H2_LEN>& c3, 
            std::vector<Element>& poly_v, 
            std::vector<Element>& poly_z) {
    size_t total_len = element_length_in_bytes(c1) + element_length_in_bytes(c2) + c3.size();
    for (auto& e : poly_v) total_len += element_length_in_bytes(e);
    for (auto& e : poly_z) total_len += element_length_in_bytes(e);

    std::vector<unsigned char> buffer(total_len);
    unsigned char* curr = buffer.data();

    auto serialize = [&](Element& e) {
        int len = element_length_in_bytes(e);
        element_to_bytes(curr, e);
        curr += len;
    };

    serialize(c1);
    serialize(c2);
    memcpy(curr, c3.data(), c3.size()); curr += c3.size();

    for (auto& e : poly_v) serialize(e);
    for (auto& e : poly_z) serialize(e);

    SHA256(buffer.data(), buffer.size(), out_buf.data());
}

inline void H4_map(CryptoContext& ctx, Element& out_Zr, 
            const std::string& id_0, const std::string& id_i, 
            Element& point) {
            
    if (!out_Zr.is_initialized()) out_Zr.init_Zr(*ctx.guard_);

    int p_len = element_length_in_bytes(point);
    std::vector<unsigned char> buffer(id_0.length() + id_i.length() + p_len);

    unsigned char* curr = buffer.data();
    memcpy(curr, id_0.c_str(), id_0.length()); curr += id_0.length();
    memcpy(curr, id_i.c_str(), id_i.length()); curr += id_i.length();
    element_to_bytes(curr, point);

    element_from_hash(out_Zr, buffer.data(), buffer.size());
}

inline bool ServerSetup(ServerContext& svc, const std::string& param_str) {
    if (param_str.empty()) {
        std::cerr << "[错误] 参数字符串为空，无法启动服务器环境。\n";
        return false;
    }

    if (!svc.guard_->initialized_) {
        std::cerr << "[错误] svc 环境未就绪" << std::endl;
        return false;
    }

    try {
        
        element_random(svc.G);     
        element_random(svc.msk);
        element_mul_zn(svc.P_pub, svc.G, svc.msk);

    } catch (const std::exception& e) {
        std::cerr << "[异常] 服务器环境构建失败: " << e.what() << std::endl;
        return false;
    }

    std::cout << "[Server] 环境原子化初始化成功，主密钥已就绪。\n";
    return true;
}


inline void ServerKeyGen(ServerContext& svc, ServerUserKeys& user, const std::string& id) {

    user.id = id;

    Element r, h1_val, tmp;
    r.init_Zr(*svc.guard_);
    h1_val.init_Zr(*svc.guard_);
    tmp.init_Zr(*svc.guard_);

    // 生成随机数 r
    element_random(r);
    if (!user.R.is_initialized()) {
        user.R.init_G1(*svc.guard_); 
    }
    if (!user.d.is_initialized()) {
        user.d.init_Zr(*svc.guard_);
    }

    // 计算 R = r * G
    element_mul_zn(user.R, svc.G, r);

    // 计算 h1 = H1(id, R)
    H1_map(svc, h1_val, id, user.R);

    // 计算 d = r + msk * h1
    element_mul(tmp, svc.msk, h1_val);
    element_add(user.d, r, tmp);

    std::cout << "[KeyGen] 成功为用户 [" << id << "] 生成私钥分量 (R, d)" << std::endl;
}

inline void ConstructPolynomial(std::vector<Element>& coeffs, std::vector<Element>& roots, Element& d, CryptoContext& ctx) {
    int n = roots.size();

    // 初始化 coeffs
    coeffs.resize(n + 1);
    for (auto& c : coeffs) {
        c.init_Zr(*ctx.guard_);
        element_set0(c);
    }

    // f(x) = 1
    element_set1(coeffs[0]);

    int degree = 0;

    for (int k = 0; k < n; k++) {
        std::vector<Element> next(degree + 2);
        for (auto& c : next) {
            c.init_Zr(*ctx.guard_);
            element_set0(c);
        }

        for (int j = 0; j <= degree + 1; j++) {
            // + coeffs[j-1]
            if (j > 0 && j - 1 <= degree) {
                element_add(next[j], next[j], coeffs[j - 1]);
            }

            // - coeffs[j] * roots[k]
            if (j <= degree) {
                Element tmp;
                tmp.init_Zr(*ctx.guard_);
                element_mul(tmp, coeffs[j], roots[k]);
                element_sub(next[j], next[j], tmp);
            }
        }

       
        for (int i = 0; i <= degree + 1; i++) {
            element_set(coeffs[i], next[i]);
        }

        degree++;
    }

    // 加 d
    element_add(coeffs[0], coeffs[0], d);
}

inline Element EvaluatePolynomial(std::vector<Element>& coeffs,Element& x,CryptoContext& ctx) {
    Element result;
    result.init_Zr(*ctx.guard_);
    element_set0(result);

    for (int i = (int)coeffs.size() - 1; i >= 0; --i) {

        element_mul(result, result, x);
        element_add(result, result, coeffs[i]);
    }
    return result;
}

inline bool Encrypt(Ciphertext& ct,UserKeys& sender,std::vector<PeerUserKeys*>& receivers,const std::vector<unsigned char>& message,CryptoContext& ctx) {

    int n = receivers.size();

    ct.clear();
    ct.C1.init_G1(*ctx.guard_);
    ct.C2.init_G1(*ctx.guard_);

    Element r, s, d1, d2;
    r.init_Zr(*ctx.guard_);
    s.init_Zr(*ctx.guard_);
    d1.init_Zr(*ctx.guard_);
    d2.init_Zr(*ctx.guard_);
    
    element_random(r);
    element_random(s);
    element_random(d1);
    element_random(d2);

    element_mul_zn(ct.C1, ctx.G, r);
    element_mul_zn(ct.C2, ctx.G, s);

    std::vector<Element> V_ids, Z_ids;

    for (int i = 0; i < n; i++) {
        Element V, Z;
        V.init_Zr(*ctx.guard_);
        Z.init_Zr(*ctx.guard_);

        Element h1, h4, tau;
        h1.init_Zr(*ctx.guard_);
        h4.init_Zr(*ctx.guard_);
        tau.init_G1(*ctx.guard_);

        Element PK1, PK2, tmp;
        PK1.init_G1(*ctx.guard_);
        PK2.init_G1(*ctx.guard_);
        tmp.init_G1(*ctx.guard_);

        // ===== V =====
        //h1(id_i,R_i)
        H1_map(ctx, h1, receivers[i]->id, receivers[i]->R);
        ///h1(id_i,R_i)P_pub
        element_mul_zn(tmp, ctx.P_pub, h1);
        //h1(id_i,R_i)P_pub+R_i
        element_add(PK1, receivers[i]->R, tmp);

        element_mul_zn(tau, receivers[i]->X, sender.x);
        tau.print("加密tau=");

        H4_map(ctx, h4, sender.id, receivers[i]->id, tau);

        element_mul_zn(PK1, PK1, h4);
        element_mul_zn(tmp, PK1, r);
        H_point_to_Zr(ctx, V, tmp);

        // ===== Z =====
        Element term_s;
        term_s.init_Zr(*ctx.guard_);
        element_mul(term_s, sender.x, h4);
        element_add(term_s, term_s, sender.d);
        element_add(term_s, term_s, s);

        element_mul_zn(PK2, receivers[i]->X, term_s);
        H_point_to_Zr(ctx, Z, PK2);

        V.print("加密V=");
        Z.print("加密Z=");

        V_ids.push_back(std::move(V));
        Z_ids.push_back(std::move(Z));
    }

    // 多项式
    ConstructPolynomial(ct.poly_coeffsV, V_ids, d1, ctx);
    ConstructPolynomial(ct.poly_coeffsZ, Z_ids, d2, ctx);

    // H2
    std::array<unsigned char, CryptoContext::H2_LEN> h2;
    H2_map(h2, ct.C1, ct.C2, d1, d2);

    size_t prefix = CryptoContext::H2_LEN - CryptoContext::MSG_LEN;

    memcpy(ct.C3.data(), h2.data(), prefix);
    for (int i = 0; i < CryptoContext::MSG_LEN; i++) {
        ct.C3[prefix + i] = message[i] ^ h2[prefix + i];
    }

    H3_map(ct.C4, ct.C1, ct.C2, ct.C3,
           ct.poly_coeffsV, ct.poly_coeffsZ);

    return true;
}

inline bool Decrypt(std::vector<unsigned char>& out,UserKeys& receiver,PeerUserKeys& sender,Ciphertext& ct,CryptoContext& ctx) {

    // H3 校验
    std::array<unsigned char, CryptoContext::H3_LEN> h3;
    H3_map(h3, ct.C1, ct.C2, ct.C3,
           ct.poly_coeffsV, ct.poly_coeffsZ);

    if (memcmp(h3.data(), ct.C4.data(), CryptoContext::H3_LEN) != 0){
        std::cout<<"完整性检验失败"<<std::endl;
        return false;
    }

    Element tau, h4, V, Z;
    tau.init_G1(*ctx.guard_);
    h4.init_Zr(*ctx.guard_);
    V.init_Zr(*ctx.guard_);
    Z.init_Zr(*ctx.guard_);

    element_mul_zn(tau, sender.X, receiver.x);
    H4_map(ctx, h4, sender.id, receiver.id, tau);

    // ===== V =====
    Element tmp;
    tmp.init_G1(*ctx.guard_);

    element_mul_zn(tmp, ct.C1, h4);
    element_mul_zn(tmp, tmp, receiver.d);
    H_point_to_Zr(ctx, V, tmp);

    // ===== Z =====
    Element term, h1;
    term.init_G1(*ctx.guard_);
    h1.init_Zr(*ctx.guard_);

    H1_map(ctx, h1, sender.id, sender.R);

    element_mul_zn(term, sender.X, h4);
    element_mul_zn(tmp, ctx.P_pub, h1);
    element_add(term, term, tmp);
    element_add(term, term, sender.R);
    element_add(term, term, ct.C2);

    element_mul_zn(term, term, receiver.x);
    H_point_to_Zr(ctx, Z, term);
    // 多项式恢复
    Element d1 = EvaluatePolynomial(ct.poly_coeffsV, V,ctx);
    Element d2 = EvaluatePolynomial(ct.poly_coeffsZ, Z,ctx);

    // H2
    std::array<unsigned char, CryptoContext::H2_LEN> h2;
    H2_map(h2, ct.C1, ct.C2, d1, d2);

    size_t prefix = CryptoContext::H2_LEN - CryptoContext::MSG_LEN;

    if (memcmp(h2.data(), ct.C3.data(), prefix) != 0)
        return false;

    out.resize(CryptoContext::MSG_LEN);
    for (int i = 0; i < CryptoContext::MSG_LEN; i++) {
        out[i] = ct.C3[prefix + i] ^ h2[prefix + i];
    }

    return true;
}