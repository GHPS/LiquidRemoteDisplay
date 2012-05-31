#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
#Warn  ; Recommended for catching common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

commandByte = %1%
if commandByte = S
 Send {Media_Stop}
 else if commandByte = N
 Send {Media_Next}
 else if commandByte = V
 Send {Media_Prev} 
 else if commandByte = H
 Send {Media_Play_Pause}
 else if commandByte = M
 Send {Volume_Mute}
 else if commandByte = K
 Send {Volume_Up 3}
 else if commandByte = J
 Send {Volume_Down 3}