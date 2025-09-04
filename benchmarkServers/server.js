const express = require("express");
const multer  = require("multer");
const path = require("path");

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, "storage/");
  },
  filename: function (req, file, cb) {
    cb(null, file.originalname);
  }
});

const upload = multer({ storage: storage });

const app = express();

app.post("/files", upload.single("file"), (req, res) => {
  if (!req.file) {
    return res.status(400).send("No file uploaded");
  }
  console.log(`Uploaded: ${req.file.filename} (${req.file.size} bytes)`);
  res.status(201).send("File uploaded successfully");
});

const PORT = 4222;
app.listen(PORT, () => {
  console.log(`Express server running at http://127.0.0.1:${PORT}`);
});
