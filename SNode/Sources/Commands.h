#ifndef COMMANDS_H_
#define COMMANDS_H_

/* Echo command expects a reply with the same command and same data it received */
#define CMD_PING_ECHO		0xDE

/* The settings (AES key, channel, address, node type, etc.) are sent with this command
 * offset,	size,	data
 * 4		4		settings hash code (used to identify if settings changed)
 * 4		16		AES Key 
 * 20		16		AES IV 
 * 36		4		Node Type (LSB first)
 * 40		2		Address (LSB first)
 * 42		1		Channel */
#define CMD_SETTINGS		0xA9

/* Instructs the node to send the storred settings hash code */
#define CMD_GET_SETTINGS_H	0xAB

/* Resets the node */
#define CMD_RESET			0x41

/* Requests the unique ID of the node */
#define CMD_GETID			0x7E

/* Notifies the node has booted and inited */
#define NT_BOOTED			0x2F

/* Sends the node id (the next 10 bytes represent the unique chip id)*/
#define NT_NODEID			0x53

/* Sends the settings hash code (4 bytes) */
#define NT_SETTINGS_H		0x3D

/* Sent as a repy to CMD_PING_ECHO */
#define NT_PING_ECHO		CMD_PING_ECHO

#define SNODE_CMD_ECHO		0xE0
#define SNODE_CMD_ECHO_R	0xE1

#endif /* COMMANDS_H_ */
