#!/usr/bin/env python3
import os
import sys
import json
import time
import subprocess
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler

class MockCloudServer(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith("/drive/v3/files"):
            self.handle_gdrive()
        elif self.path.startswith("/mock-azure-container?restype=container&comp=list") or "comp=list" in self.path:
            self.handle_azure()
        elif self.path.startswith("/mock-s3-bucket"):
            self.handle_s3()
        elif self.path == "/":
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"OK")
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Not Found")

    def handle_gdrive(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        
        if "pageToken" not in self.path:
            # Page 1
            data = {
                "nextPageToken": "page2_token",
                "files": [
                    {"id": "file1_id", "name": "drive_file_1.txt", "size": "100", "modifiedTime": "2026-03-01T12:00:00.000Z", "mimeType": "text/plain"},
                    {"id": "dir1_id", "name": "drive_dir", "mimeType": "application/vnd.google-apps.folder"}
                ]
            }
        else:
            # Page 2
            data = {
                "files": [
                    {"id": "file2_id", "name": "drive_file_2.jpg", "size": "2048", "modifiedTime": "2026-03-02T12:00:00.000Z", "mimeType": "image/jpeg"}
                ]
            }
        self.wfile.write(json.dumps(data).encode('utf-8'))

    def handle_s3(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/xml')
        self.end_headers()
        
        if "continuation-token" not in self.path:
            # Page 1
            xml = """<?xml version="1.0" encoding="UTF-8"?>
            <ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
                <Name>mock-s3-bucket</Name>
                <IsTruncated>true</IsTruncated>
                <NextContinuationToken>token2</NextContinuationToken>
                <Contents>
                    <Key>s3_file_1.txt</Key>
                    <LastModified>2026-03-01T12:00:00.000Z</LastModified>
                    <Size>500</Size>
                </Contents>
                <Contents>
                    <Key>folder1/</Key>
                    <Size>0</Size>
                </Contents>
            </ListBucketResult>
            """
        else:
            # Page 2
            xml = """<?xml version="1.0" encoding="UTF-8"?>
            <ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
                <Name>mock-s3-bucket</Name>
                <IsTruncated>false</IsTruncated>
                <Contents>
                    <Key>s3_file_2.png</Key>
                    <LastModified>2026-03-02T12:00:00.000Z</LastModified>
                    <Size>4096</Size>
                </Contents>
            </ListBucketResult>
            """
        self.wfile.write(xml.encode('utf-8'))

    def handle_azure(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/xml')
        self.end_headers()
        
        if "marker" not in self.path:
            xml = """<?xml version="1.0" encoding="utf-8"?>
            <EnumerationResults ServiceEndpoint="http://localhost:8080/devstoreaccount1" ContainerName="mock-azure-container">
                <Blobs>
                    <Blob>
                        <Name>azure_file_1.txt</Name>
                        <Properties>
                            <Last-Modified>Sun, 01 Mar 2026 12:00:00 GMT</Last-Modified>
                            <Content-Length>800</Content-Length>
                        </Properties>
                    </Blob>
                </Blobs>
                <NextMarker>token2</NextMarker>
            </EnumerationResults>"""
        else:
            xml = """<?xml version="1.0" encoding="utf-8"?>
            <EnumerationResults ServiceEndpoint="http://localhost:8080/devstoreaccount1" ContainerName="mock-azure-container">
                <Blobs>
                    <Blob>
                        <Name>azure_file_2.pdf</Name>
                        <Properties>
                            <Last-Modified>Mon, 02 Mar 2026 12:00:00 GMT</Last-Modified>
                            <Content-Length>1500</Content-Length>
                        </Properties>
                    </Blob>
                </Blobs>
                <NextMarker />
            </EnumerationResults>"""
        self.wfile.write(xml.encode('utf-8'))

def run_server(port):
    server = HTTPServer(('localhost', port), MockCloudServer)
    server.serve_forever()

if __name__ == "__main__":
    PORT = 18080
    server_thread = threading.Thread(target=run_server, args=(PORT,), daemon=True)
    server_thread.start()
    
    # Wait for server to bound
    time.sleep(1)
    
    # We assume fo_cli.exe is in build/cli/Debug/fo_cli.exe or build/cli/fo_cli.exe
    exe_path = None
    possible_paths = [
        "build/cli/Debug/fo_cli.exe",
        "build/cli/fo_cli.exe",
        "build/cli/Release/fo_cli.exe",
    ]
    for p in possible_paths:
        if os.path.exists(p):
            exe_path = p
            break
            
    if not exe_path:
        print("Could not find fo_cli.exe. Ensure project is built.")
        sys.exit(1)

    print(f"Testing with {exe_path}")

    # 1. Test GDrive
    env = os.environ.copy()
    env["GDRIVE_API_BASE"] = f"http://localhost:{PORT}"
    
    print("=== Testing Google Drive Scanner ===")
    cmd = [exe_path, "scan", "--scanner=gdrive", "--gdrive-token=mock_token", "--format=json", "gdrive://"]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    try:
        data = json.loads(res.stdout)
        assert len(data) == 2, f"Expected 2 GDrive files, got {len(data)}"
        print("GDrive OK")
    except Exception as e:
        print(f"GDrive Failed: {e}\nStdout: {res.stdout}\nStderr: {res.stderr}")
        sys.exit(1)

    # 2. Test S3
    env["S3_ENDPOINT_URL"] = f"http://localhost:{PORT}"
    
    print("=== Testing S3 Scanner ===")
    cmd = [exe_path, "scan", "--scanner=s3", "--s3-bucket=mock-s3-bucket", "--format=json", "s3://mock-s3-bucket/"]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    try:
        data = json.loads(res.stdout)
        # S3 scanner outputs 2 files: folder marker was skipped natively
        assert len(data) == 2, f"Expected 2 S3 files, got {len(data)}"
        print("S3 OK")
    except Exception as e:
        print(f"S3 Failed: {e}\nStdout: {res.stdout}\nStderr: {res.stderr}")
        sys.exit(1)

    # 3. Test Azure
    conn_str = f"DefaultEndpointsProtocol=http;AccountName=devstoreaccount1;AccountKey=Eby8;BlobEndpoint=http://127.0.0.1:{PORT}/devstoreaccount1;"
    
    print("=== Testing Azure Scanner ===")
    cmd = [exe_path, "scan", "--scanner=azure", "--azure-connection=" + conn_str, "--azure-container=mock-azure-container", "--format=json", "azure://mock-azure-container/"]
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    try:
        data = json.loads(res.stdout)
        assert len(data) == 2, f"Expected 2 Azure files, got {len(data)}"
        print("Azure OK")
    except Exception as e:
        print(f"Azure Failed: {e}\nStdout: {res.stdout}\nStderr: {res.stderr}")
        sys.exit(1)

    print("\nAll integration tests passed successfully.")
    sys.exit(0)
