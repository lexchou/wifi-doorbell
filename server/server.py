#!/usr/bin/env python3.5
import asyncio



clients = []


class DoorbellServerProtocol(asyncio.Protocol):
    def connection_made(self, transport):
        self.transport = transport
        clients.append(self)
    def connection_lost(self, exc):
        clients.remove(self)

    def data_received(self,data):
        message = data.decode()
        print("Data received:" + message)
        for c in clients:
            if c == self:
                continue
            c.transport.write(data)
            

loop = asyncio.get_event_loop()
coro = loop.create_server(DoorbellServerProtocol, '0.0.0.0', 3001)
server = loop.run_until_complete(coro)
try:
    loop.run_forever()
except KeyboardInterrupt:
    pass

# Close the server
server.close()
loop.run_until_complete(server.wait_closed())
loop.close()
