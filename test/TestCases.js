const should = require("should");
const shell = require("shelljs");
const path = require("path");
const fs = require("fs");
const net = require("net");
const child_process = require("child_process");
const util = require("util");
const yaml = require("js-yaml");
const delayClient = require("./lib/DelayClient");

const ExecPath = path.resolve("npserver_single_proc");
const WorkspaceDir = path.resolve("test/_workspace");
const CommandDelayTime = 250;

let serverPortStart = 12300;

describe("TestCases", function() {
  shell.cd(__dirname);

  shell
    .ls("-l", "test-cases")
    .filter(g => g && g.name.charAt(0) != "_" && g.isDirectory())
    .forEach(group => {
      describe(group.name, function() {
        let groupDir = path.resolve("test-cases", group.name);
        shell.mkdir("-p", path.join(groupDir, "_template"));
        shell.exec("make", {
          silent: true,
          cwd: path.join(groupDir, "_prepare")
        });
        const groupWorkspace = path.resolve(WorkspaceDir, group.name);

        shell
          .ls("-l", groupDir)
          .filter((c, i) => c && c.name.charAt(0) != "_" && c.isDirectory())
          .forEach(test => {
            const testWorkspace = path.join(groupWorkspace, test.name);
            shell.mkdir("-p", testWorkspace);
            shell.cp("-r", path.join(groupDir, "_template/*"), testWorkspace);

            let serverPort = serverPortStart++;
            let shellServerProcess = child_process.spawn(
              ExecPath,
              [serverPort],
              {
                silent: true,
                cwd: testWorkspace
              }
            );

            let testDir = path.join(groupDir, test.name);
            it(test.name, async function() {
              this.slow(10000);
              this.timeout(0);
              let commands = yaml.safeLoad(
                await util.promisify(fs.readFile)(
                  path.join(testDir, "in.yaml"),
                  "utf8"
                ),
                {
                  schema: yaml.FAILSAFE_SCHEMA
                }
              );
              let answer = yaml.safeLoad(
                await util.promisify(fs.readFile)(
                  path.join(testDir, "out.yaml"),
                  "utf8"
                ),
                {
                  schema: yaml.FAILSAFE_SCHEMA
                }
              );

              let result = await delayClient(
                "localhost",
                serverPort,
                commands,
                CommandDelayTime
              );
              shellServerProcess.kill();
              result.should.be.eql(answer);

              // let output = yaml.safeDump(result, {
              //   indent: 2,
              //   lineWidth: 256,
              //   noRefs: true,
              //   schema: yaml.FAILSAFE_SCHEMA
              // })
              // await util.promisify(fs.writeFile)(path.join(testDir, "out.yaml"), output);
            });
          });

        after(() => {
          shell.rm("-Rf", groupWorkspace);
        });
      });
    });
});
