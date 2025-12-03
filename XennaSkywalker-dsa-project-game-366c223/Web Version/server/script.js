// ---------------------------------------
// 1. INITIALIZATION & SETUP
// ---------------------------------------
const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");
const messageDiv = document.getElementById("message");

// Game Settings
const TILE_SIZE = 30; // Size of each grid block in pixels
const FPS = 20; // <<< CHANGE FPS HERE
const FRAME_DELAY = 1000 / FPS;

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
      console.error(`Failed to load: ${name}.png — check /assets folder`);
    };

    assets[name] = img;
  });
}

loadAssets();

// ---------------------------------------
// 3. INPUT HANDLING
// ---------------------------------------
function sendInput(key, choiceId = -1) {
  fetch("/input", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ key, choiceId }),
  }).catch((err) => console.error("Input Error:", err));
}

document.addEventListener("keydown", (e) => {
  if (["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"].includes(e.code)) {
    e.preventDefault();
  }

  if (e.key === "ArrowLeft") sendInput("left");
  else if (e.key === "ArrowRight") sendInput("right");
  else if (e.key === "ArrowUp") sendInput("up");
  else if (e.key.toLowerCase() === "s") sendInput("save");
  else if (e.key.toLowerCase() === "u") sendInput("undo");
  else if (e.key.toLowerCase() === "e") sendInput("replay");
  else if (e.key === "1") sendInput("choose", 1);
  else if (e.key === "2") sendInput("choose", 2);
});

// ---------------------------------------
// 4. DRAWING & RENDERING
// ---------------------------------------
function drawGame(data) {
  if (!data || !data.grid) return;

  // Clear Screen
  ctx.fillStyle = "#000";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // Background
  if (assets["background"]?.complete) {
    ctx.drawImage(assets["background"], 0, 0, canvas.width, canvas.height);
  }

  // Grid Tiles
  for (let y = 0; y < data.height; y++) {
    const row = data.grid[y];

    for (let x = 0; x < data.width; x++) {
      const char = row[x];
      const posX = x * TILE_SIZE;
      const posY = y * TILE_SIZE;

      if (char === "#") {
        ctx.drawImage(assets["wall"], posX, posY, TILE_SIZE, TILE_SIZE);
      } else if (char === "P") {
        ctx.drawImage(assets["player"], posX, posY, TILE_SIZE, TILE_SIZE);
      } else if (char === "G") {
        ctx.drawImage(assets["goal"], posX, posY, TILE_SIZE, TILE_SIZE);
      } else if (char === "D") {
        ctx.drawImage(assets["door"], posX, posY, TILE_SIZE, TILE_SIZE);
      }
    }
  }
}

// ---------------------------------------
// 5. GAME LOOP (FPS LIMITED)
// ---------------------------------------
async function update() {
  try {
    const res = await fetch("/state");
    const data = await res.json();

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
    } else {
      messageDiv.textContent = "";
    }

    drawGame(data);
  } catch (err) {
    console.error("Game Loop Error:", err);
  }

  // run next frame after a delay
  setTimeout(update, FRAME_DELAY);
}

update();
