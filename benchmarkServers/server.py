from flask import Flask, request
import os

app = Flask(__name__)
UPLOAD_FOLDER = "storage"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/files", methods=["POST"])
def upload_file():
    if "file" not in request.files:
        return "No file part", 400

    file = request.files["file"]
    if file.filename == "":
        return "No selected file", 400

    save_path = os.path.join(UPLOAD_FOLDER, file.filename)
    file.save(save_path)

    print(f"Uploaded: {file.filename} ({os.path.getsize(save_path)} bytes)")
    return "File uploaded successfully", 201

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=4223)
