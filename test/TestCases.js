const should = require("should");
const shell = require("shelljs");
const path = require("path");
const fs = require("fs");
const net = require("net");
const child_process = require("child_process");
const util = require("util");
const yaml = require("js-yaml");
const psTree = require("ps-tree");
const tcpPortUsed = require("tcp-port-used");
const delayClient = require("./lib/DelayClient");

const ExecPath = path.resolve("np_simple");
const ExecCheckShm = path.resolve("test/bin/shm.sh");
const ExecCheckZombie = path.resolve("test/bin/zombie.sh");

const WorkspaceDir = path.resolve("test/_workspace");

const Mode = process.env.TEST_MODE;
const CommandDelayTime = process.env.DELAY || 200;
const ServerPort = parseInt(process.env.PORT) || 12300;
const ServerStartDelay = process.env.SERVER_START_DELAY || 100;
const ServerStopDelay = process.env.SERVER_END_DELAY || 100;

const sleep = t => new Promise(r => setTimeout(() => r(), t || 100));

shell.cd(__dirname);
shell
  .ls("-l", "test-cases")
  .filter(g => g && g.name.charAt(0) != "_" && g.isDirectory())
  .forEach(group => {
    describe(group.name, function() {
      const groupDir = path.resolve("test-cases", group.name);
      shell.mkdir("-p", path.join(groupDir, "_template"));
      shell.exec("make", {
        silent: true,
        cwd: path.join(groupDir, "_prepare")
      });
      const groupWorkspace = path.resolve(WorkspaceDir, group.name);

      shell
        .ls("-l", groupDir)
        .filter(c => c && c.name.charAt(0) != "_" && c.isDirectory())
        .forEach((test, i) => {
          describe(test.name, function() {
            let serverProcess;
            const testDir = path.join(groupDir, test.name);
            const testWorkspace = path.join(groupWorkspace, test.name);

            before(async function() {
              this.enableTimeouts(false);

              shell.mkdir("-p", testWorkspace);
              shell.cp("-r", path.join(groupDir, "_template/*"), testWorkspace);

              await tcpPortUsed.waitUntilFree(ServerPort, 500);
              serverProcess = child_process.spawn(ExecPath, [ServerPort], {
                silent: true,
                cwd: testWorkspace
              });
              await sleep(ServerStartDelay);
            });

            it("correct-output", async function() {
              this.slow(20000);
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
              let answerYAML = await util.promisify(fs.readFile)(
                path.join(testDir, "out.yaml"),
                "utf8"
              );
              let answer =
                Mode === "yaml"
                  ? answerYAML
                  : yaml.safeLoad(answerYAML, {
                      schema: yaml.FAILSAFE_SCHEMA
                    });

              let result = await (Mode === "yaml"
                ? delayClient.yaml
                : delayClient)(
                "127.0.0.1",
                ServerPort,
                commands,
                CommandDelayTime
              );

              result.should.be.eql(answer);
            });

            it("no-zombie-processes", async function() {
              this.slow(5000);
              this.timeout(0);

              let children = await util.promisify(psTree)(serverProcess.pid);
              serverProcess.kill();
              await sleep(ServerStopDelay);

              let existsZombieProcesses = false;
              children.forEach(c => {
                try {
                  process.kill(c.PID, 9);
                  existsZombieProcesses = true;
                } catch (e) {
                  if (e.code === "EPERM") {
                    existsZombieProcesses = true;
                  }
                }
              });
              existsZombieProcesses.should.be.eql(false);
            });
          });
        });

      after(function() {
        shell.rm("-Rf", groupWorkspace);
      });
    });
  });
