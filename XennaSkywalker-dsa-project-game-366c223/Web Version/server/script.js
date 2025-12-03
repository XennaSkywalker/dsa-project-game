// Game Configuration
const TILE_SIZE = 30;
const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");
const messageDiv = document.getElementById("message");
const choicesDiv = document.getElementById("choices");

// Assets
const assets = {};
const assetNames = ["background", "wall", "platform", "player", "goal", "top"];

// Load Images
let imagesLoaded = 0;
function loadAssets() {
    assetNames.forEach(name => {
        const img = new Image();
        img.src = `/assets/${name}.png`;
        img.onload = () => {
            imagesLoaded++;
            console.log(`Loaded: ${name}`);
        };
        assets[name] = img;
    });
}
loadAssets();

// Input Handling
function sendInput(key, choiceId = null) {
    fetch("/input", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ key, choiceId }),
    }).catch(err => console.error("Input Error:", err));
}

// Keyboard Listeners
document.addEventListener("keydown", (e) => {
    if (e.key === "ArrowLeft") sendInput("left");
    else if (e.key === "ArrowRight") sendInput("right");
    else if (e.key === "ArrowUp") sendInput("up");
    else if (e.key.toLowerCase() === "s") sendInput("save");
    else if (e.key.toLowerCase() === "u") sendInput("undo");
    else if (e.key.toLowerCase() === "r") sendInput("replay");
});

// Main Draw Function
function drawGame(data) {
    if (!data || !data.grid) return;

    // Clear Canvas
    ctx.fillStyle = "#000";
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw Background (tiled)
    if (assets["background"]) {
        ctx.drawImage(assets["background"], 0, 0, canvas.width, canvas.height);
    }

    // Draw Grid
    for (let y = 0; y < data.height; y++) {
        const row = data.grid[y];
        for (let x = 0; x < data.width; x++) {
            const char = row[x];
            const posX = x * TILE_SIZE;
            const posY = y * TILE_SIZE;

            if (char === '#') {
                if (assets["wall"]) ctx.drawImage(assets["wall"], posX, posY, TILE_SIZE, TILE_SIZE);
                else { ctx.fillStyle = "gray"; ctx.fillRect(posX, posY, TILE_SIZE, TILE_SIZE); }
            } 
            else if (char === 'P') {
                if (assets["player"]) ctx.drawImage(assets["player"], posX, posY, TILE_SIZE, TILE_SIZE);
                else { ctx.fillStyle = "red"; ctx.fillRect(posX, posY, TILE_SIZE, TILE_SIZE); }
            }
            else if (char === 'G') {
                if (assets["goal"]) ctx.drawImage(assets["goal"], posX, posY, TILE_SIZE, TILE_SIZE);
                else { ctx.fillStyle = "yellow"; ctx.fillRect(posX, posY, TILE_SIZE, TILE_SIZE); }
            }
            else if (char === 'D') {
                // Door visualization
                ctx.fillStyle = "cyan"; 
                ctx.fillRect(posX, posY, TILE_SIZE, TILE_SIZE);
            }
        }
    }
}

// Game Loop
async function update() {
    try {
        const res = await fetch("/state");
        const data = await res.json();

        // 1. Tutorial Message
        if (data.tutorial) {
            messageDiv.textContent = `TUTORIAL: ${data.tutorial}`;
        } else {
            messageDiv.textContent = "";
        }

        // 2. Choices UI
        if (data.choices && data.choices.length > 0) {
            choicesDiv.innerHTML = "<h3>CHOOSE A PATH:</h3>";
            data.choices.forEach(c => {
                const btn = document.createElement("button");
                btn.innerText = c.text;
                btn.onclick = () => sendInput("choose", c.id);
                choicesDiv.appendChild(btn);
            });
        } else {
            choicesDiv.innerHTML = "";
        }

        drawGame(data);
    } catch (err) {
        console.error("Game Loop Error:", err);
    }
    requestAnimationFrame(update);
}

// Start
update();