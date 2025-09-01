function updateClock() {
  const now = new Date();
  const timeString = now.toLocaleTimeString();
  document.getElementById("clock").textContent = timeString;
}

setInterval(updateClock, 1000);
updateClock();

// Fetch and render file list from /getAllFiles
async function fetchFiles() {
  try {
    const res = await fetch('/getAllFiles');
    let files = [];
    const contentType = res.headers.get('content-type') || '';
    if (contentType.includes('application/json')) {
      files = await res.json();
    } else {
      const text = await res.text();
      files = text.split('\n').map(s => s.trim()).filter(Boolean).map(p => (p.startsWith('/') ? p : ('/' + p)));
    }

    const list = document.getElementById('file-list');
    if (!list) return;
    list.innerHTML = '';
    let pathPrefix = '/files';
    files.forEach(path => {
      const li = document.createElement('li');
      const span = document.createElement('span');
      span.textContent = path;
      const a = document.createElement('a');
      a.className = 'download-btn';
      a.href = pathPrefix + path;
      a.download = '';
      a.textContent = 'Download';
      li.appendChild(span);
      li.appendChild(a);
      list.appendChild(li);
    });
  } catch (e) {
    console.error('Failed to fetch files', e);
  }
}

// Intercept upload form to submit via fetch and refresh file list in-place
document.addEventListener('DOMContentLoaded', () => {
  const form = document.querySelector('.upload-form');
  if (form) {
    form.addEventListener('submit', async (ev) => {
      ev.preventDefault();
      const fd = new FormData(form);
      try {
        const res = await fetch(form.action || '/files', {
          method: form.method || 'POST',
          body: fd
        });
        if (!res.ok) {
          const t = await res.text();
          alert('Upload failed: ' + t);
          return;
        }
        // success -> refresh file list and reset form
        await fetchFiles();
        form.reset();
      } catch (e) {
        console.error(e);
        alert('Upload error');
      }
    });
  }

  // initial population
  fetchFiles();
});