# DelayClient Commands Script
YAML Version: 1.2   
Processed by any YAML processors in failsafe schema.

## Command format
```
- <client_name>: <command: !!str>
```
Send <command> to server through client <client_name>.   
If <client_name> does not exist, this command would be ignored.

## Builtin function
### Builtin function format
```
- <builtin_functions>: <parameters>
```
### Builtin functions

#### `- _CONNECT: <client_name: !!str>`

Create a new client named `<client_name>` and connect to server.   
If `<client_name>` already exists, this command would be ignored.

#### `- _DISCONNECT: <client_name: !!str>`

Disconnect `<client_name>` from server and delete the client. The client will be ended immediately and won't receive any messages after executing this command.   
If `<client_name>` does not exist, this command would be ignored.

#### `- _SLEEP: <time(ms): !!int>`
Do nothing and wait `<time>`

## Example
```yaml
- _CONNECT: u1
- _CONNECT: u2
- u1: echo helloworld
- u2: cat LARGE_FILE
- _SLEEP: 1000
- u1: exit
- u2: exit
```
