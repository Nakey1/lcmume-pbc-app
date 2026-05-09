#include "httplib.h"
#include "lcmume.hpp"
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

std::string param =
        "type a\n"
        "q 87807107996633125224377819847540498158068831994142082"
        "1102865339926647563088022295707862517942266222142315585"
        "8769582317459277713367317481324925129998224791\n"
        "h 12016012264891146079388821366740534204802954401251311"
        "822919615131047207289359704531102844802183906537786776\n"
        "r 730750818665451621361119245571504901405976559617\n"
        "exp2 159\n"
        "exp1 107\n"
        "sign1 1\n"
        "sign0 1\n";


// 全局状态管理
enum class UserStatus {
    INIT = 0,
    PARTIAL_KEY_ISSUED,
    PUBKEY_UPLOADED,
    REVOKED
};

struct UserRecord {
    std::shared_ptr<ServerUserKeys> keys;
    UserStatus status;
};

// 全局注册表与线程安全锁
std::unordered_map<std::string, UserRecord> user_registry;
std::mutex user_registry_mutex;

// 使用智能指针管理全局密码学上下文
std::unique_ptr<ServerContext> g_svc = nullptr;

int main() {
    
    try {

        g_svc = std::make_unique<ServerContext>(param);
        ServerSetup(*g_svc,param); 
        std::cout << "[KGC] 现代密码学上下文初始化完成 (RAII)。" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Fatal] 初始化失败: " << e.what() << std::endl;
        return -1;
    }

    httplib::Server svr;


    // 接口 1: 获取公共参数
    svr.Get("/get_public_params", [](const httplib::Request&, httplib::Response& res) {
        json j;
        j["status"] = "success";
        j["param_str"] = g_svc->get_param_str(); // 从 context 获取参数串
        j["G"] = g_svc->G.to_string();           // 封装好的字符串转换接口
        j["P_pub"] = g_svc->P_pub.to_string();

        res.set_content(j.dump(), "application/json");
        std::cout << "[KGC] 已分发公共参数。" << std::endl;
    });

    // 接口 2: 生成部分私钥 
    svr.Get("/generate_partial_key", [](const httplib::Request& req, httplib::Response& res) {
        std::string user_id = req.get_param_value("user_id");
        if (user_id.empty()) {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Missing user_id\"}", "application/json");
            return;
        }

        std::shared_ptr<ServerUserKeys> new_user = nullptr;
        {
            std::lock_guard<std::mutex> lock(user_registry_mutex);  

            auto it = user_registry.find(user_id);
            if (it != user_registry.end()) {
                if (it->second.status == UserStatus::REVOKED) {
                    res.status = 403;
                    res.set_content("{\"status\":\"error\",\"message\":\"User revoked\"}", "application/json");
                    return;
                }
                if (it->second.status != UserStatus::INIT) {
                    res.status = 409; // Conflict
                    res.set_content("{\"status\":\"error\",\"message\":\"Key already issued\"}", "application/json");
                    return;
                }
                new_user = it->second.keys;
            } else {
                new_user = std::make_shared<ServerUserKeys>();
                user_registry[user_id] = {new_user, UserStatus::INIT};
            }

            // 传入 Context 
            ServerKeyGen(*g_svc, *new_user, user_id); 
            user_registry[user_id].status = UserStatus::PARTIAL_KEY_ISSUED;
        }

        json j;
        j["status"] = "success";
        j["partial_key_d"] = new_user->d.to_string();
        j["partial_key_R"] = new_user->R.to_string();
        res.set_content(j.dump(), "application/json");
        std::cout << "[KGC] 已为用户 " << user_id << " 生成部分私钥。" << std::endl;
    });

   
    // 接口 3: 上传公钥
    svr.Post("/upload_pubkey", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body); 
            std::string user_id = j.value("user_id", "");
            std::string X_str = j.value("X", "");

            if (user_id.empty() || X_str.empty()) {
                res.status = 400;
                return;
            }

            std::lock_guard<std::mutex> lock(user_registry_mutex);
            if (user_registry.count(user_id) == 0 || user_registry[user_id].status == UserStatus::INIT) {
                res.status = 403; 
                res.set_content("{\"status\":\"error\",\"message\":\"Registration flow error\"}", "application/json");
                return;
            }

            // 更新用户公钥并切换状态

            auto& user_keys = user_registry[user_id].keys;
            
            if (!user_keys->X.is_initialized()) {
                user_keys->X.init_G1(*g_svc->guard_);
            }

            element_set_str(user_registry[user_id].keys->X, X_str.c_str(), 10);
            user_registry[user_id].status = UserStatus::PUBKEY_UPLOADED;

            res.set_content("{\"status\":\"success\"}", "application/json");
            std::cout << "[KGC] 用户 " << user_id << " 已上传公钥私钥。" << std::endl;
        } catch (...) {
            res.status = 400;
        }
    });

    
    // 接口 4: 查询目标用户公钥 
    svr.Post("/get_user_pubkeys", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string target_id = j.value("target_id", "");

            std::lock_guard<std::mutex> lock(user_registry_mutex);
            if (user_registry.count(target_id) == 0 || user_registry[target_id].status != UserStatus::PUBKEY_UPLOADED) {
                res.status = 404;
                res.set_content("{\"status\":\"error\",\"message\":\"Target not available\"}", "application/json");
                return;
            }

            auto& target_keys = user_registry[target_id].keys;
            json response;
            response["status"] = "success";
            response["target_public_key_R"] = target_keys->R.to_string();
            response["target_public_key_X"] = target_keys->X.to_string();

            res.status = 200; 
            res.set_content(response.dump(), "application/json");

        } catch (...) {
            res.status = 400;
        }
    });

    std::cout << "[KGC Server] 监听端口 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);

    return 0;
}