const should = require("should");
const shell = require("shelljs");
const path = require("path");
const fs = require("fs");
const net = require("net");
const child_process = require("child_process");
const util = require("util");

const execPath = path.resolve("npshell_server");
const workSpaceDir = path.resolve("test/_workSpace");

let serverPortStart = 12300;

describe("TestCases", () => {
  shell.cd("test");

  shell
    .ls("-l", "cases")
    .filter(c => c && c.name.charAt(0) != "_")
    .forEach(item => {
      describe(item.name, () => {
        let testItemDir = path.resolve("cases", item.name);
        shell.mkdir("-p", path.join(testItemDir, "_workSpaceTemplate"));
        shell.exec("make", {
          silent: true,
          cwd: path.join(testItemDir, "_workSpacePrepare")
        });
        const itemWorkSpace = path.resolve(workSpaceDir, item.name);

        shell
          .ls("-l", testItemDir)
          .filter((c, i) => c && c.name.charAt(0) != "_")
          .forEach(c => {
            const caseWorkSpace = path.join(itemWorkSpace, c.name);
            shell.mkdir("-p", caseWorkSpace);
            shell.cp(
              "-R",
              path.join(testItemDir, "_workSpaceTemplate/*"),
              caseWorkSpace
            );

            let serverPort = serverPortStart++;
            let shellServerProcess = child_process.spawn(
              execPath,
              [serverPort],
              {
                silent: true,
                cwd: caseWorkSpace
              }
            );

            let testCaseDir = path.join(testItemDir, c.name);
            it(c.name, async () => {
              let client = new net.Socket();
              client.connect(serverPort, "localhost", () => {
                client.write(fs.readFileSync(path.join(testCaseDir, "in.txt")));
                setTimeout(() => {
                  client.destroy();
                }, 9000);
              });

              let shellOutput = "";
              client.on("data", data => {
                shellOutput += data;
              });

              return new Promise(resolve => {
                client.on("close", () => {
                  shellServerProcess.kill();
                  shellOutput.should.be.eql(
                    fs
                      .readFileSync(path.join(testCaseDir, "out.txt"))
                      .toString()
                  );
                  resolve();
                });
              });
            }).timeout(10000);
          });

        after(() => {
          shell.rm("-Rf", itemWorkSpace);
        });
      });
    });
});
