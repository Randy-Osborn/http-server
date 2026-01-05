// Phase 3: Static File Server Test JavaScript
console.log("✅ JavaScript file loaded successfully!");
console.log("📁 File: staticFile.js");
console.log(
  "🌐 This proves the server can serve .js files with correct MIME type"
);

// Add dynamic content to the page
const target = document.getElementById("js-target");
if (target) {
  target.innerHTML = `
        <h3 class="success">✅ JavaScript Executed!</h3>
        <p>This content was dynamically added by <code>staticFile.js</code></p>
        <p>The server correctly served the JavaScript file with <code>Content-Type: application/javascript</code></p>
        <p><strong>Current time:</strong> ${new Date().toLocaleString()}</p>
    `;
}
