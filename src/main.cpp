#include <unistd.h>

#include "server/webserver.h"

int main() {
  /* 守护进程 后台运行 */
  // daemon(1, 0);

  WebServer server(
      1316,        // 端口
      3,           // ET模式
      60000,       // 超时时限
      false,       // 设置优雅退出
      3306,        // 数据库端口号
      "root",      // 数据库用户名
      "root",      // 数据库密码
      "webserver", // 数据库名
      12,          // 数据库连接池大小
      6,           // 线程池数量
      true,        // 日志开关
      1,           // 日志等级
      1024);       // 日志异步队列容量
  server.Start();
}
