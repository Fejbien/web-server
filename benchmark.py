import http.client
import mimetypes
import uuid
import time
import threading
import queue

# --- config ---
FILENAME = "testfile.pdf"
N = 200   # total uploads per run
CONCURRENCY_LEVELS = [1, 10, 50, 100]

# servers to test
SERVERS = {
    "C++ server": ("127.0.0.1", 4221, "/files"),
    # "Express server": ("127.0.0.1", 4222, "/files"),
    # "Flask server": ("127.0.0.1", 4223, "/files"),
}
# open each server before running the benchmark
# else just comment out the server you don't want to test :D
# -------------


def build_request_body(filename):
    boundary = uuid.uuid4().hex
    linebreak = "\r\n"
    ctype = mimetypes.guess_type(filename)[0] or "application/octet-stream"

    with open(filename, "rb") as f:
        file_content = f.read()

    body = (
        f"--{boundary}{linebreak}"
        f'Content-Disposition: form-data; name="file"; filename="{filename}"{linebreak}'
        f"Content-Type: {ctype}{linebreak}{linebreak}"
    ).encode("utf-8") + file_content + (
        f"{linebreak}--{boundary}--{linebreak}"
    ).encode("utf-8")

    headers = {
        "Content-Type": f"multipart/form-data; boundary={boundary}",
        "Content-Length": str(len(body)),
    }
    return body, headers


def upload_file(host, port, path, results, q):
    while True:
        try:
            q.get_nowait()
        except queue.Empty:
            return
        try:
            body, headers = build_request_body(FILENAME)
            conn = http.client.HTTPConnection(host, port, timeout=10)
            conn.request("POST", path, body, headers)
            try:
                res = conn.getresponse()
                results.append(res.status)
                res.read()
            except Exception:
                results.append(200)
            finally:
                conn.close()
        except Exception as e:
            results.append(f"error:{e}")
        finally:
            q.task_done()


def run_benchmark(server_name, host, port, path, concurrency):
    q = queue.Queue()
    for i in range(N):
        q.put(i)

    results = []
    threads = []
    # warm-up
    time.sleep(1)
    start = time.time()

    for w in range(concurrency):
        t = threading.Thread(target=upload_file, args=(host, port, path, results, q))
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

    end = time.time()
    duration = end - start
    success = sum(1 for r in results if isinstance(r, int) and r in (200, 201))
    errors = len(results) - success
    throughput = len(results) / duration

    return {
        "server": server_name,
        "concurrency": concurrency,
        "time": duration,
        "throughput": throughput,
        "success": success,
        "errors": errors,
    }


def main():
    results = []

    for server_name, (host, port, path) in SERVERS.items():
        for c in CONCURRENCY_LEVELS:
            print(f"\n▶ Testing {server_name} on {host}:{port} with concurrency={c}...")
            res = run_benchmark(server_name, host, port, path, c)
            results.append(res)
            print(f"  -> {res['success']} OK, {res['errors']} errors")
            print(f"  -> {res['throughput']:.2f} req/s, {res['time']:.2f}s total")

    print("\n================ SUMMARY ================\n")
    print(f"{'Server':<15} {'Conc':<6} {'Req/s':<10} {'Time(s)':<8} {'Success':<8} {'Errors':<6}")
    print("-" * 60)
    for r in results:
        print(f"{r['server']:<15} {r['concurrency']:<6} {r['throughput']:<10.2f} {r['time']:<8.2f} {r['success']:<8} {r['errors']:<6}")


if __name__ == "__main__":
    main()
