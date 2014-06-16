% -*- fill-column: 58; -*-

:- module(rpc_command_v, []).

/** <module> An altcoin RPC command.
*/

new_class(rpc_command_v, object_v,
	       [command]).

new_class(getinfo_v, rpc_command_v, []).

