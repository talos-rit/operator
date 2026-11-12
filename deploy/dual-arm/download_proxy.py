"""Temporary loopback-only package-download proxy for an offline Pi over SSH."""
import select
import socket
import socketserver
from urllib.parse import urlsplit


def allowed(host):
    return any(host == domain or host.endswith('.' + domain) for domain in (
        'docker.com', 'docker.io', 'cloudflare.docker.com', 'cloudflarestorage.com',
        'debian.org', 'raspberrypi.com', 'raspberrypi.org', 'python.org', 'pypi.org', 'pythonhosted.org', 'github.com',
        'githubusercontent.com', 'githubassets.com',
    ))


class Proxy(socketserver.BaseRequestHandler):
    def handle(self):
        upstream = None
        try:
            header = b''
            while b'\r\n\r\n' not in header and len(header) < 65536:
                chunk = self.request.recv(4096)
                if not chunk:
                    return
                header += chunk
            first, rest = header.split(b'\r\n', 1)
            method, target, version = first.decode('ascii').split()
            if method == 'CONNECT':
                host, port = target.rsplit(':', 1)
                port = int(port)
                if port != 443 or not allowed(host):
                    raise ValueError('destination not allowed')
                upstream = socket.create_connection((host, port), timeout=20)
                self.request.sendall(b'HTTP/1.1 200 Connection Established\r\n\r\n')
            else:
                url = urlsplit(target)
                if url.scheme != 'http' or not allowed(url.hostname):
                    raise ValueError('destination not allowed')
                upstream = socket.create_connection((url.hostname, url.port or 80), timeout=20)
                path = (url.path or '/') + ('?' + url.query if url.query else '')
                upstream.sendall(f'{method} {path} {version}\r\n'.encode() + rest)
            self.request.settimeout(None)
            upstream.settimeout(None)
            while True:
                readable, _, _ = select.select([self.request, upstream], [], [], 90)
                if not readable:
                    return
                for source in readable:
                    data = source.recv(65536)
                    if not data:
                        return
                    (upstream if source is self.request else self.request).sendall(data)
        except (OSError, ValueError):
            pass
        finally:
            if upstream is not None:
                upstream.close()


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True


if __name__ == '__main__':
    with Server(('127.0.0.1', 18081), Proxy) as server:
        server.serve_forever()
