const should = require("should");
const shell = require("shelljs");
const path = require("path");
const fs = require("fs");

const execPath = path.resolve("build/npshell");
const workSpaceDir = path.resolve("test/_workSpace");

describe("TestCases", () => {
  shell.cd("test");

  shell.ls("-l", "cases").forEach(item => {
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

          let testCaseDir = path.join(testItemDir, c.name);
          it(c.name, () => {
            shell
              .exec(`${execPath} < ${path.join(testCaseDir, "in.txt")}`, {
                silent: true,
                cwd: caseWorkSpace
              })
              .stdout.should.be.eql(
                fs.readFileSync(path.join(testCaseDir, "out.txt")).toString()
              );
          }).timeout(10000);
        });

      after(function() {
        shell.rm("-Rf", itemWorkSpace);
      });
    });
  });
});
