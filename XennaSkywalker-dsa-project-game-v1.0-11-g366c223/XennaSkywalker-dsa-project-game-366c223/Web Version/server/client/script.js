// ... (Keep your existing canvas/context variables)

// Add a div in your HTML for UI or create it dynamically
// Assuming you have <div id="ui-layer"></div> in index.html
// If not, just alert for now or log to console

function sendInput(key, choiceId = null) {
  fetch("/input", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ key, choiceId }),
  });
}

async function update(time = 0) {
  // ... (Keep your delta time logic)

    try {
      const res = await fetch("/state");
      const data = await res.json();
      
      // 1. Render Tutorial Message
      if (data.tutorial) {
          messageDiv.textContent = "TUTORIAL: " + data.tutorial; // Reusing your flash message div
      } else {
          messageDiv.textContent = ""; 
      }

      // 2. Render Choices (Doors)
      // This is a simple implementation. A real game would overlay buttons.
      if (data.choices && data.choices.length > 0) {
          let choiceText = "DOOR FOUND! Press 1 or 2:\n";
          data.choices.forEach(c => {
              choiceText += `[${c.id}] ${c.text}\n`;
          });
          // Display this somewhere prominent
          console.log(choiceText); 
          // For simple testing, we rely on keyboard listeners 1 and 2 below
      }

      drawGame(data);
    } catch (err) {
      console.error(err);
    }
  // ... (Keep animation frame request)
}

// Updated Keyboard Controls
document.addEventListener("keydown", (e) => {
  if (e.key === "ArrowLeft") sendInput("left");
  else if (e.key === "ArrowRight") sendInput("right");
  else if (e.key === "ArrowUp") sendInput("up");
  // ... (Save/Undo/Replay logic)
  
  // New: Choice selection
  else if (e.key === "1") sendInput("choose", 1);
  else if (e.key === "2") sendInput("choose", 2);
});