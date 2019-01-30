const _ = require("lodash");
const fs = require("fs");
const net = require("net");
const util = require("util");
const yaml = require("js-yaml");

const sleep = t => new Promise(r => setTimeout(() => r(), t || 100));

async function delayClient(ip, port, commands, delay) {
  let clients = {};
  let result = [];
  let receivedMessage = {};

  const builtinFunctions = {
    _CONNECT: name =>
      new Promise(res => {
        if (clients[name]) {
          return;
        }

        let client = new net.Socket();
        clients[name] = client;
        client.on("data", data => {
          receivedMessage[name] =
            (receivedMessage[name] || "") + data.toString();
        });

        client.on("close", () => {
          delete clients[name];
        });

        client.connect(port, ip, () => {
          res();
        });
      }),
    _DISCONNECT: name =>
      new Promise(res => {
        if (!clients[name]) {
          return;
        }
        clients[name].destroy();
        clients[name].end(() => {
          res();
        });
      }),
    _SLEEP: time => sleep(parseInt(time))
  };

  let executeCommand = async cmd => {
    let key = Object.keys(cmd)[0];
    let val = cmd[key];
    receivedMessage = {};

    if (builtinFunctions[key]) {
      await builtinFunctions[key](val);
    } else if (clients[key]) {
      clients[key].write(val + "\n");
    }

    if (key == "_SLEEP") {
      Object.keys(receivedMessage).forEach(u => {
        result[result.length - 1].RECV[u] =
          (result[result.length - 1].RECV[u] || "") + receivedMessage[u];
      });
    } else {
      result.push({
        COMMAND: {
          [key]: val
        },
        RECV: receivedMessage
      });
    }
  };

  for (cmd of commands) {
    await executeCommand(cmd);
    await sleep(delay);
  }

  for (client of Object.values(clients)) {
    client.destroy();
    client.end();
  }

  return result.map(cmd => ({
    ...cmd,
    RECV: _.flatten(
      Object.keys(cmd.RECV)
        .sort()
        .map(cname =>
          cmd.RECV[cname].split("\n").map(line => ({
            [cname]: line
          }))
        )
    )
  }));
}
delayClient.yaml = async (ip, port, commands, delay) => {
  return yaml.safeDump(await delayClient(ip, port, commands, delay), {
    indent: 2,
    lineWidth: 240,
    noRefs: true,
    schema: yaml.FAILSAFE_SCHEMA
  });
};
module.exports = delayClient;
