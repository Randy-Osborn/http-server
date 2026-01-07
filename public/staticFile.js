// Phase 4: Enhanced Error Handling HTTP Server
console.log(" JavaScript file loaded successfully!");
console.log(" File: staticFile.js");
console.log(" Phase 4: Enhanced Error Handling Active");
console.log(
  " Server Features: Custom error pages, logging, and static file serving"
);

// Add dynamic content to the page
const target = document.getElementById("js-target");
if (target) {
  const serverStartTime = new Date();

  target.innerHTML = `
        <h3 class="success">✅ JavaScript Module Loaded!</h3>
        <p>This dynamic content was injected by <code>staticFile.js</code></p>
        <p>The server correctly served the JavaScript file with <code>Content-Type: application/javascript</code></p>
        <p><strong>Page loaded at:</strong> ${serverStartTime.toLocaleString()}</p>
        <p><strong>Server phase:</strong> Phase 4 - Enhanced Error Handling</p>
    `;
}

// Test 404 error page
window.testError404 = function () {
  console.log(" Testing 404 error...");
  window.open("/nonexistent.html", "_blank");
};

// Log when the page is fully loaded
window.addEventListener("load", () => {
  console.log(" All resources loaded successfully!");
  console.log(" Phase 4 HTTP Server is fully operational");
});
