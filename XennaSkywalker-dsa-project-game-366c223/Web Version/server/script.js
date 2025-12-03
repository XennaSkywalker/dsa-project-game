// ---------------------------------------
// 1. INITIALIZATION & SETUP
// ---------------------------------------
const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");
const messageDiv = document.getElementById("message");

const TILE_SIZE = 30;
const FPS = 20;
const FRAME_DELAY = 1000 / FPS;

// ⭐ Game stop flag
let gameOver = false;

// ⭐ Flash message timeout
let flashTimeout = null;

// ---------------------------------------
// 2. ASSET LOADING
// ---------------------------------------
const assets = {};
const assetNames = ["background", "wall", "platform", "player", "goal", "door"];
let imagesLoaded = 0;

function loadAssets() {
  assetNames.forEach((name) => {
    const img = new Image();
    img.src = `/assets/${name}.png`;

    img.onload = () => {
      imagesLoaded++;
      console.log(`Loaded image: ${name}`);
    };
    img.onerror = () => {
      console.error(`Failed to load: ${name}.png`);
    };

    assets[name] = img;
  });
}

loadAssets();

// ---------------------------------------
// 3. FLASH MESSAGE
// ---------------------------------------
function flashMessage(text, color = "#0ff", duration = 800) {
  messageDiv.textContent = text;
  messageDiv.style.color = color;

  if (flashTimeout) clearTimeout(flashTimeout);

  flashTimeout = setTimeout(() => {
    messageDiv.textContent = "";
    flashTimeout = null;
  }, duration);
}

// ---------------------------------------
// 4. INPUT HANDLING
// ---------------------------------------
function sendInput(key, choiceId = -1) {
  fetch("/input", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ key, choiceId }),
  }).catch((err) => console.error("Input Error:", err));
}

document.addEventListener("keydown", (e) => {
  if (gameOver) return;

  if (["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"].includes(e.code)) {
    e.preventDefault();
  }

  if (e.key === "ArrowLeft") sendInput("left");
  else if (e.key === "ArrowRight") sendInput("right");
  else if (e.key === "ArrowUp") sendInput("up");
  else if (e.key.toLowerCase() === "s") {
    sendInput("save");
    flashMessage("Game Saved!", "rgba(255, 0, 144, 1)");
  } else if (e.key.toLowerCase() === "u") {
    sendInput("undo");
    flashMessage("Undo Performed!", "#ff0");
  } else if (e.key.toLowerCase() === "e") {
    sendInput("replay");
    flashMessage("Replay Started!", "#f0f");
  } else if (e.key === "1") sendInput("choose", 1);
  else if (e.key === "2") sendInput("choose", 2);
  else if (e.key.toLowerCase() === "q") sendInput("reset");
});

// ---------------------------------------
// 5. DRAWING & RENDERING
// ---------------------------------------
function drawGame(data) {
  if (!data || !data.grid) return;

  ctx.fillStyle = "#000";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  if (assets["background"]?.complete) {
    ctx.drawImage(assets["background"], 0, 0, canvas.width, canvas.height);
  }

  for (let y = 0; y < data.height; y++) {
    const row = data.grid[y];

    for (let x = 0; x < data.width; x++) {
      const char = row[x];
      const posX = x * TILE_SIZE;
      const posY = y * TILE_SIZE;

      if (char === "#")
        ctx.drawImage(assets["wall"], posX, posY, TILE_SIZE, TILE_SIZE);
      else if (char === "P")
        ctx.drawImage(assets["player"], posX, posY, TILE_SIZE, TILE_SIZE);
      else if (char === "G")
        ctx.drawImage(assets["goal"], posX, posY, TILE_SIZE, TILE_SIZE);
      else if (char === "D")
        ctx.drawImage(assets["door"], posX, posY, TILE_SIZE, TILE_SIZE);
    }
  }
}

// ---------------------------------------
// 6. GAME LOOP
// ---------------------------------------
async function update() {
  if (gameOver) return;

  try {
    const res = await fetch("/state");
    const data = await res.json();

    if (data.goalMessage && data.goalMessage !== "") {
      messageDiv.textContent = data.goalMessage;
      messageDiv.style.color = "rgba(255, 0, 0, 1)";
      gameOver = true;
      return;
    }

    if (data.tutorial && data.tutorial !== "") {
      messageDiv.textContent = "TUTORIAL: " + data.tutorial;
      messageDiv.style.color = "#ff0";
    } else if (data.choices && data.choices.length > 0) {
      let choiceMsg = "DECISION TIME! Press ";
      data.choices.forEach((c) => {
        choiceMsg += `[${c.id}] for ${c.text}   `;
      });
      messageDiv.textContent = choiceMsg;
      messageDiv.style.color = "#0ff";
    } else if (!flashTimeout) {
      // Only clear message if no flash message active
      messageDiv.textContent = "";
    }

    drawGame(data);
  } catch (err) {
    console.error("Game Loop Error:", err);
  }

  setTimeout(update, FRAME_DELAY);
}

update();
