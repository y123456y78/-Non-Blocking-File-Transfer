#Non-Blocking-File-Transfer  

This is a pratice project of Intro. to Network Programming.
====================================
***Recommended: If you are taking this course, please solve it on yourself first.***  

A user can upload his files on the server.  
The clients of the user are running on different hosts.  
The server will make sure all clients of the user have same files in their directory.  
When a new client connects to the server, the server should transmit all the files, which have been uploaded by the other clients to the new client immediately.  

./make run  
`Compile client and server program`  
./server &lt;port&gt;  
`Run the server on specific port`  
./client &ltip> &ltport&gt; &ltusername&gt;  
`login the client as different User`  
Client can enter two command to upload a file and sleep  
put &ltfilename&gt;  
sleep &ltsleep second&gt;  
