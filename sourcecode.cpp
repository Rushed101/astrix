/*
 * ============================================================
 * Single-File Minecraft Cheat Client (C++/Java Hybrid Style)
 * ============================================================
 * Expert C++/Java Developer Implementation
 * Lightweight, Modular, Asynchronous Design
 * All features in ONE self-contained file
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <queue>
#include <memory>

// ============================================================
// SECTION 1: CROSS-PLATFORM ABSTRACTIONS & HOOKS
// ============================================================

namespace Minecraft {
    // Simulated Minecraft game state (replace with actual memory reads/hooks)
    struct GameState {
        struct Entity {
            float x, y, z;
            float yaw, pitch;
            float health;
            float distance;
            bool visible;
            std::string name;
            bool isFriend;
            bool isTeam;
            bool isVehicle;
        };
        
        struct LocalPlayer {
            float x, y, z;
            float yaw, pitch;
            float health;
            bool onGround;
            bool inVehicle;
            std::string currentWeapon;
        };
        
        LocalPlayer localPlayer;
        std::vector<Entity> entities;
        float fps = 60.0f;
        int tickCounter = 0;
    };
    
    // Simulated rendering context
    struct RenderContext {
        int screenWidth = 1920;
        int screenHeight = 1080;
        float deltaTime = 0.016f;
    };
}

// ============================================================
// SECTION 2: UTILITY CLASSES
// ============================================================

namespace Utility {
    class Timer {
    private:
        std::chrono::steady_clock::time_point startTime;
    public:
        Timer() { reset(); }
        void reset() { startTime = std::chrono::steady_clock::now(); }
        float elapsedMillis() {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        }
        bool hasPassed(float ms) { return elapsedMillis() >= ms; }
    };
    
    class RandomGenerator {
    private:
        std::mt19937 rng;
    public:
        RandomGenerator() : rng(std::random_device{}()) {}
        int nextInt(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(rng);
        }
        float nextFloat(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(rng);
        }
    };
    
    class Math {
    public:
        static float lerp(float a, float b, float t) { return a + (b - a) * t; }
        static float clamp(float v, float min, float max) { return std::max(min, std::min(max, v)); }
        static float distance2D(float x1, float y1, float x2, float y2) {
            return std::sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
        }
        static float distance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
            return std::sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
        }
        static float angleDifference(float a, float b) {
            float diff = fmod(b - a, 360.0f);
            if (diff > 180.0f) diff -= 360.0f;
            if (diff < -180.0f) diff += 360.0f;
            return diff;
        }
    };
}

// ============================================================
// SECTION 3: EVENT BUS SYSTEM (Zero-overhead)
// ============================================================

class EventBus {
private:
    std::unordered_map<int, std::vector<std::function<void()>>> listeners;
    std::mutex mutex;
    
public:
    enum EventType {
        TICK, RENDER, KEY_INPUT, MOUSE_INPUT, WORLD_LOAD
    };
    
    void subscribe(int eventType, std::function<void()> callback) {
        std::lock_guard<std::mutex> lock(mutex);
        listeners[eventType].push_back(callback);
    }
    
    void publish(int eventType) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = listeners.find(eventType);
        if (it != listeners.end()) {
            for (auto& cb : it->second) {
                cb();
            }
        }
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        listeners.clear();
    }
};

// ============================================================
// SECTION 4: CONFIGURATION SYSTEM (JSON-like in single file)
// ============================================================

class ConfigManager {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> configs;
    std::string currentProfile = "default";
    
    // Feature settings
    struct FeatureSettings {
        bool enabled = false;
        float value = 0.0f;
        int keybind = -1;
        std::string mode = "default";
    };
    
    std::unordered_map<std::string, FeatureSettings> features;
    
public:
    ConfigManager() {
        // Initialize default features
        features["aimbot"] = FeatureSettings();
        features["killaura"] = FeatureSettings();
        features["esp"] = FeatureSettings();
        features["triggerbot"] = FeatureSettings();
        features["speedwalk"] = FeatureSettings();
        features["boatfly"] = FeatureSettings();
        
        // Default values
        features["aimbot"].value = 5.0f; // FOV
        features["killaura"].value = 4.5f; // Range
        features["speedwalk"].value = 1.5f; // Speed multiplier
        features["boatfly"].value = 1.0f; // Vertical speed
        
        loadProfile("default");
    }
    
    void setFeatureEnabled(const std::string& feature, bool enabled) {
        features[feature].enabled = enabled;
    }
    
    bool isFeatureEnabled(const std::string& feature) {
        return features[feature].enabled;
    }
    
    void setFeatureValue(const std::string& feature, float value) {
        features[feature].value = value;
    }
    
    float getFeatureValue(const std::string& feature) {
        return features[feature].value;
    }
    
    void setKeybind(const std::string& feature, int key) {
        features[feature].keybind = key;
    }
    
    int getKeybind(const std::string& feature) {
        return features[feature].keybind;
    }
    
    // Profile management
    void saveProfile(const std::string& name) {
        std::ostringstream oss;
        for (auto& [key, setting] : features) {
            oss << key << ":" << setting.enabled << ":" 
                << setting.value << ":" << setting.keybind << ";";
        }
        configs[name] = {{"data", oss.str()}};
        currentProfile = name;
        saveToFile(name);
    }
    
    void loadProfile(const std::string& name) {
        if (configs.find(name) == configs.end()) {
            // Create default
            configs[name] = {};
            saveProfile(name);
            return;
        }
        
        auto& config = configs[name];
        if (config.find("data") != config.end()) {
            std::string data = config["data"];
            std::stringstream ss(data);
            std::string item;
            while (std::getline(ss, item, ';')) {
                if (item.empty()) continue;
                std::stringstream itemStream(item);
                std::string key, enabled, value, keybind;
                std::getline(itemStream, key, ':');
                std::getline(itemStream, enabled, ':');
                std::getline(itemStream, value, ':');
                std::getline(itemStream, keybind, ':');
                
                if (features.find(key) != features.end()) {
                    features[key].enabled = (enabled == "1");
                    features[key].value = std::stof(value);
                    features[key].keybind = std::stoi(keybind);
                }
            }
        }
        currentProfile = name;
    }
    
    void saveToFile(const std::string& name) {
        std::ofstream file("client_config_" + name + ".json");
        if (file.is_open()) {
            file << "{\n";
            file << "  \"profile\": \"" << name << "\",\n";
            file << "  \"features\": {\n";
            bool first = true;
            for (auto& [key, setting] : features) {
                if (!first) file << ",\n";
                file << "    \"" << key << "\": {\n";
                file << "      \"enabled\": " << (setting.enabled ? "true" : "false") << ",\n";
                file << "      \"value\": " << setting.value << ",\n";
                file << "      \"keybind\": " << setting.keybind << "\n";
                file << "    }";
                first = false;
            }
            file << "\n  }\n}";
            file.close();
        }
    }
    
    void loadFromFile(const std::string& name) {
        std::ifstream file("client_config_" + name + ".json");
        if (!file.is_open()) return;
        // Simplified parsing - in production would use actual JSON parser
        file.close();
        loadProfile(name);
    }
    
    void deleteProfile(const std::string& name) {
        if (configs.find(name) != configs.end()) {
            configs.erase(name);
            if (currentProfile == name) {
                currentProfile = "default";
                loadProfile("default");
            }
        }
    }
    
    std::vector<std::string> getProfiles() {
        std::vector<std::string> names;
        for (auto& [name, _] : configs) {
            names.push_back(name);
        }
        return names;
    }
    
    void renameProfile(const std::string& oldName, const std::string& newName) {
        if (configs.find(oldName) != configs.end() && configs.find(newName) == configs.end()) {
            configs[newName] = configs[oldName];
            configs.erase(oldName);
            if (currentProfile == oldName) {
                currentProfile = newName;
            }
        }
    }
    
    std::string getCurrentProfile() { return currentProfile; }
};

// ============================================================
// SECTION 5: CORE FEATURE IMPLEMENTATIONS
// ============================================================

class CheatFeatures {
private:
    ConfigManager& config;
    EventBus& eventBus;
    Minecraft::GameState& gameState;
    Minecraft::RenderContext& renderContext;
    Utility::RandomGenerator rng;
    
    // Aimbot state
    struct AimbotState {
        bool isLocking = false;
        float targetYaw = 0.0f;
        float targetPitch = 0.0f;
        int targetIndex = -1;
        Utility::Timer smoothTimer;
    } aimbotState;
    
    // KillAura state
    struct KillAuraState {
        Utility::Timer attackTimer;
        int currentTarget = -1;
        std::vector<int> targets;
    } killauraState;
    
    // ESP state
    struct ESPState {
        std::vector<Minecraft::GameState::Entity> renderCache;
        Utility::Timer updateTimer;
    } espState;
    
    // Triggerbot state
    struct TriggerbotState {
        Utility::Timer delayTimer;
        bool isHolding = false;
    } triggerbotState;
    
    // Speedwalk state
    struct SpeedwalkState {
        float currentSpeed = 0.0f;
    } speedwalkState;
    
    // Boatfly state
    struct BoatflyState {
        float verticalVelocity = 0.0f;
        float horizontalVelocity = 0.0f;
    } boatflyState;
    
public:
    CheatFeatures(ConfigManager& cfg, EventBus& bus, Minecraft::GameState& state, 
                  Minecraft::RenderContext& ctx)
        : config(cfg), eventBus(bus), gameState(state), renderContext(ctx) {
        
        // Subscribe to events
        eventBus.subscribe(EventBus::TICK, [this]() { onTick(); });
        eventBus.subscribe(EventBus::RENDER, [this]() { onRender(); });
        eventBus.subscribe(EventBus::KEY_INPUT, [this]() { onKeyInput(); });
    }
    
    // ============================================================
    // 1. AIMBOT IMPLEMENTATION
    // ============================================================
    void updateAimbot() {
        if (!config.isFeatureEnabled("aimbot")) {
            aimbotState.isLocking = false;
            return;
        }
        
        float fov = config.getFeatureValue("aimbot");
        float smooth = 0.15f; // Hardcoded smooth factor
        
        // Find best target
        int bestIndex = -1;
        float bestScore = fov + 1.0f;
        
        for (int i = 0; i < gameState.entities.size(); i++) {
            auto& entity = gameState.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isFriend || entity.isTeam) continue;
            
            // Calculate angle to entity
            float dx = entity.x - gameState.localPlayer.x;
            float dy = entity.y - gameState.localPlayer.y;
            float dz = entity.z - gameState.localPlayer.z;
            
            float targetYaw = atan2(dz, dx) * 180.0f / 3.14159f;
            float targetPitch = atan2(dy, sqrt(dx*dx + dz*dz)) * 180.0f / 3.14159f;
            
            float yawDiff = Utility::Math::angleDifference(gameState.localPlayer.yaw, targetYaw);
            float pitchDiff = Utility::Math::angleDifference(gameState.localPlayer.pitch, targetPitch);
            float angleDist = sqrt(yawDiff*yawDiff + pitchDiff*pitchDiff);
            
            if (angleDist < fov && angleDist < bestScore) {
                bestScore = angleDist;
                bestIndex = i;
                
                // Store target angles
                aimbotState.targetYaw = targetYaw;
                aimbotState.targetPitch = targetPitch;
            }
        }
        
        if (bestIndex != -1) {
            // Smooth interpolation
            float currentYaw = gameState.localPlayer.yaw;
            float currentPitch = gameState.localPlayer.pitch;
            
            float smoothYaw = Utility::Math::lerp(currentYaw, aimbotState.targetYaw, smooth);
            float smoothPitch = Utility::Math::lerp(currentPitch, aimbotState.targetPitch, smooth);
            
            // Apply smoothed angles
            gameState.localPlayer.yaw = smoothYaw;
            gameState.localPlayer.pitch = smoothPitch;
            
            aimbotState.isLocking = true;
            aimbotState.targetIndex = bestIndex;
        } else {
            aimbotState.isLocking = false;
            aimbotState.targetIndex = -1;
        }
    }
    
    // ============================================================
    // 2. KILLAURA IMPLEMENTATION
    // ============================================================
    void updateKillAura() {
        if (!config.isFeatureEnabled("killaura")) {
            killauraState.targets.clear();
            return;
        }
        
        float range = config.getFeatureValue("killaura");
        int maxTargets = 1; // Single target mode
        bool multiTarget = false;
        
        // Find targets in range
        std::vector<int> validTargets;
        for (int i = 0; i < gameState.entities.size(); i++) {
            auto& entity = gameState.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isFriend || entity.isTeam) continue;
            
            float dist = Utility::Math::distance3D(
                gameState.localPlayer.x, gameState.localPlayer.y, gameState.localPlayer.z,
                entity.x, entity.y, entity.z
            );
            
            if (dist <= range) {
                validTargets.push_back(i);
            }
        }
        
        // APS/CPS control
        float attackDelay = 100.0f + rng.nextInt(0, 200); // Randomized 100-300ms
        if (killauraState.attackTimer.hasPassed(attackDelay)) {
            if (!validTargets.empty()) {
                // Attack targets
                if (multiTarget) {
                    for (int idx : validTargets) {
                        attackEntity(idx);
                    }
                } else {
                    // Single target - closest
                    int closest = validTargets[0];
                    float closestDist = INFINITY;
                    for (int idx : validTargets) {
                        float dist = Utility::Math::distance3D(
                            gameState.localPlayer.x, gameState.localPlayer.y, gameState.localPlayer.z,
                            gameState.entities[idx].x, gameState.entities[idx].y, gameState.entities[idx].z
                        );
                        if (dist < closestDist) {
                            closestDist = dist;
                            closest = idx;
                        }
                    }
                    attackEntity(closest);
                }
                killauraState.attackTimer.reset();
            }
        }
    }
    
    void attackEntity(int index) {
        // Simulated attack
        if (index >= 0 && index < gameState.entities.size()) {
            gameState.entities[index].health -= 0.5f; // Reduced damage
            if (gameState.entities[index].health < 0) {
                gameState.entities[index].health = 0;
            }
        }
    }
    
    // ============================================================
    // 3. ESP IMPLEMENTATION
    // ============================================================
    void updateESP() {
        if (!config.isFeatureEnabled("esp")) return;
        
        // Update render cache every 100ms
        if (espState.updateTimer.hasPassed(100)) {
            espState.renderCache.clear();
            for (auto& entity : gameState.entities) {
                if (entity.health > 0 && entity.visible) {
                    espState.renderCache.push_back(entity);
                }
            }
            espState.updateTimer.reset();
        }
    }
    
    void renderESP() {
        if (!config.isFeatureEnabled("esp")) return;
        
        for (auto& entity : espState.renderCache) {
            // 2D Bounding Box (simplified - in production would use actual 3D->2D projection)
            float screenX = renderContext.screenWidth / 2 + (entity.x - gameState.localPlayer.x) * 50;
            float screenY = renderContext.screenHeight / 2 - (entity.y - gameState.localPlayer.y) * 50;
            float size = 30.0f * (1.0f / (1.0f + entity.distance * 0.1f));
            
            // Draw box (simulated rendering)
            drawBox(screenX - size, screenY - size, size * 2, size * 2, 0.0f, 1.0f, 0.0f);
            
            // Draw tracer line from center
            drawLine(renderContext.screenWidth/2, renderContext.screenHeight/2, 
                     screenX, screenY, 1.0f, 0.0f, 0.0f);
            
            // Health bar
            float healthBarX = screenX - size;
            float healthBarY = screenY + size + 5;
            float healthBarWidth = size * 2;
            float healthBarHeight = 4;
            float healthPercent = entity.health / 20.0f;
            
            drawBox(healthBarX, healthBarY, healthBarWidth * healthPercent, healthBarHeight, 
                    0.0f, 1.0f, 0.0f);
            drawBox(healthBarX + healthBarWidth * healthPercent, healthBarY, 
                    healthBarWidth * (1.0f - healthPercent), healthBarHeight, 
                    1.0f, 0.0f, 0.0f);
            
            // Draw name and distance
            std::string info = entity.name + " " + std::to_string((int)entity.distance) + "m";
            drawText(screenX - 20, screenY - size - 15, info, 1.0f, 1.0f, 1.0f);
        }
    }
    
    // ============================================================
    // 4. TRIGGERBOT IMPLEMENTATION
    // ============================================================
    void updateTriggerbot() {
        if (!config.isFeatureEnabled("triggerbot")) return;
        
        bool isHolding = triggerbotState.isHolding; // Would check actual key state
        
        if (isHolding) {
            // Check if crosshair is over entity
            int targetIndex = getCrosshairTarget();
            if (targetIndex != -1) {
                if (triggerbotState.delayTimer.hasPassed(config.getFeatureValue("triggerbot"))) {
                    attackEntity(targetIndex);
                    triggerbotState.delayTimer.reset();
                }
            }
        }
    }
    
    int getCrosshairTarget() {
        // Simplified crosshair target detection
        float fov = 2.0f; // Very small FOV for crosshair
        int best = -1;
        float bestDist = fov + 1.0f;
        
        for (int i = 0; i < gameState.entities.size(); i++) {
            auto& entity = gameState.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isFriend || entity.isTeam) continue;
            
            float dx = entity.x - gameState.localPlayer.x;
            float dy = entity.y - gameState.localPlayer.y;
            float dz = entity.z - gameState.localPlayer.z;
            
            float targetYaw = atan2(dz, dx) * 180.0f / 3.14159f;
            float targetPitch = atan2(dy, sqrt(dx*dx + dz*dz)) * 180.0f / 3.14159f;
            
            float yawDiff = Utility::Math::angleDifference(gameState.localPlayer.yaw, targetYaw);
            float pitchDiff = Utility::Math::angleDifference(gameState.localPlayer.pitch, targetPitch);
            float angleDist = sqrt(yawDiff*yawDiff + pitchDiff*pitchDiff);
            
            if (angleDist < fov && angleDist < bestDist) {
                bestDist = angleDist;
                best = i;
            }
        }
        return best;
    }
    
    // ============================================================
    // 5. SPEEDWALK IMPLEMENTATION
    // ============================================================
    void updateSpeedwalk() {
        if (!config.isFeatureEnabled("speedwalk")) return;
        
        float multiplier = config.getFeatureValue("speedwalk");
        bool groundOnly = true; // Configurable
        
        if (gameState.localPlayer.onGround || !groundOnly) {
            // Modify movement speed
            float baseSpeed = 0.1f;
            speedwalkState.currentSpeed = baseSpeed * multiplier;
            
            // Apply to game state (would hook movement in real implementation)
            // This is a simulation
        }
    }
    
    // ============================================================
    // 6. BOATFLY IMPLEMENTATION
    // ============================================================
    void updateBoatfly() {
        if (!config.isFeatureEnabled("boatfly")) return;
        
        if (!gameState.localPlayer.inVehicle) return;
        
        float verticalSpeed = config.getFeatureValue("boatfly");
        float horizontalSpeed = config.getFeatureValue("boatfly") * 0.5f;
        bool glideMode = false;
        
        // Simulate flight
        if (glideMode) {
            // Smooth descent
            boatflyState.verticalVelocity = -0.01f;
        } else {
            // Allow vertical control
            // In real implementation, would modify vehicle motion
            boatflyState.verticalVelocity = verticalSpeed * 0.1f;
        }
        
        // Apply horizontal movement
        // In real implementation, would modify vehicle motion
        boatflyState.horizontalVelocity = horizontalSpeed * 0.1f;
    }
    
    // ============================================================
    // RENDER HELPERS (Simulated Direct OpenGL Hooks)
    // ============================================================
    void drawBox(float x, float y, float w, float h, float r, float g, float b) {
        // Simulated OpenGL rendering
        // In production, would use actual GL calls or MC rendering hooks
        // This is a placeholder
    }
    
    void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {
        // Simulated line drawing
    }
    
    void drawText(float x, float y, const std::string& text, float r, float g, float b) {
        // Simulated text rendering
    }
    
    // ============================================================
    // EVENT HANDLERS
    // ============================================================
    void onTick() {
        // Update all features in tick loop (60fps)
        if (gameState.tickCounter % 2 == 0) { // Every 2 ticks for performance
            updateAimbot();
            updateKillAura();
            updateTriggerbot();
            updateSpeedwalk();
            updateBoatfly();
            updateESP();
        }
        gameState.tickCounter++;
    }
    
    void onRender() {
        // Render ESP and UI elements
        renderESP();
    }
    
    void onKeyInput() {
        // Handle keybind toggles
        // Simplified - would check actual key presses
        for (auto& [name, feature] : config.features) {
            if (feature.keybind != -1) {
                // Check if key is pressed and toggle
                // feature.enabled = !feature.enabled;
            }
        }
    }
};

// ============================================================
// SECTION 6: GUI SYSTEM (ClickGUI with Modern Design)
// ============================================================

class ClickGUI {
private:
    ConfigManager& config;
    Minecraft::RenderContext& renderContext;
    bool isOpen = false;
    int selectedTab = 0;
    std::vector<std::string> tabs = {"Combat", "Movement", "Render", "Settings", "Config"};
    
    // UI State
    struct SliderState {
        float value;
        bool dragging = false;
    };
    std::unordered_map<std::string, SliderState> sliders;
    
    struct ToggleState {
        bool enabled;
    };
    std::unordered_map<std::string, ToggleState> toggles;
    
    // Animation state
    float animProgress = 0.0f;
    Utility::Timer animTimer;
    
public:
    ClickGUI(ConfigManager& cfg, Minecraft::RenderContext& ctx) 
        : config(cfg), renderContext(ctx) {
        
        // Initialize UI states from config
        for (auto& [name, feature] : config.features) {
            toggles[name].enabled = feature.enabled;
            sliders[name].value = feature.value;
        }
    }
    
    void toggle() {
        isOpen = !isOpen;
        if (isOpen) {
            animProgress = 0.0f;
            animTimer.reset();
        }
    }
    
    void render() {
        if (!isOpen) return;
        
        // Animation
        float animDuration = 200.0f; // ms
        if (!animTimer.hasPassed(animDuration)) {
            animProgress = animTimer.elapsedMillis() / animDuration;
            animProgress = std::min(animProgress, 1.0f);
            // Ease out cubic
            animProgress = 1.0f - pow(1.0f - animProgress, 3);
        } else {
            animProgress = 1.0f;
        }
        
        // Main window
        float windowWidth = 400;
        float windowHeight = 500;
        float windowX = (renderContext.screenWidth - windowWidth) / 2;
        float windowY = (renderContext.screenHeight - windowHeight) / 2;
        
        // Scale animation
        float scale = 0.9f + 0.1f * animProgress;
        float alpha = 0.8f + 0.2f * animProgress;
        
        // Draw background (dark theme)
        drawRect(windowX, windowY, windowWidth, windowHeight, 0.07f, 0.07f, 0.09f, alpha);
        drawRectBorder(windowX, windowY, windowWidth, windowHeight, 0.2f, 0.2f, 0.3f, alpha);
        
        // Title bar
        drawRect(windowX, windowY, windowWidth, 40, 0.1f, 0.1f, 0.15f, alpha);
        drawText(windowX + 20, windowY + 10, "Client v2.0", 0.8f, 0.8f, 1.0f);
        
        // Close button
        drawRect(windowX + windowWidth - 30, windowY + 5, 25, 25, 0.2f, 0.1f, 0.1f, alpha);
        drawText(windowX + windowWidth - 22, windowY + 8, "X", 1.0f, 0.3f, 0.3f);
        
        // Tabs
        float tabX = windowX + 10;
        float tabY = windowY + 45;
        float tabWidth = (windowWidth - 20) / tabs.size();
        float tabHeight = 30;
        
        for (int i = 0; i < tabs.size(); i++) {
            bool isSelected = (i == selectedTab);
            drawRect(tabX + i * tabWidth, tabY, tabWidth - 2, tabHeight, 
                    isSelected ? 0.2f : 0.1f, 
                    isSelected ? 0.1f : 0.08f, 
                    isSelected ? 0.3f : 0.12f, 
                    alpha);
            drawText(tabX + i * tabWidth + 5, tabY + 8, tabs[i], 
                    isSelected ? 0.8f : 0.5f, 
                    isSelected ? 0.6f : 0.5f, 
                    isSelected ? 1.0f : 0.5f);
        }
        
        // Content area
        float contentX = windowX + 15;
        float contentY = windowY + 85;
        float contentWidth = windowWidth - 30;
        float contentHeight = windowHeight - 100;
        
        // Render tab content
        switch (selectedTab) {
            case 0: renderCombatTab(contentX, contentY, contentWidth, contentHeight, alpha); break;
            case 1: renderMovementTab(contentX, contentY, contentWidth, contentHeight, alpha); break;
            case 2: renderRenderTab(contentX, contentY, contentWidth, contentHeight, alpha); break;
            case 3: renderSettingsTab(contentX, contentY, contentWidth, contentHeight, alpha); break;
            case 4: renderConfigTab(contentX, contentY, contentWidth, contentHeight, alpha); break;
        }
    }
    
    void renderCombatTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        renderToggle(x, y + yOffset, "Aimbot", "aimbot", alpha); yOffset += 35;
        renderSlider(x, y + yOffset, "FOV", "aimbot", 0, 90, alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "KillAura", "killaura", alpha); yOffset += 35;
        renderSlider(x, y + yOffset, "Range", "killaura", 3, 6, alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Triggerbot", "triggerbot", alpha); yOffset += 35;
        renderSlider(x, y + yOffset, "Delay (ms)", "triggerbot", 0, 500, alpha);
    }
    
    void renderMovementTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        renderToggle(x, y + yOffset, "Speedwalk", "speedwalk", alpha); yOffset += 35;
        renderSlider(x, y + yOffset, "Speed Multiplier", "speedwalk", 1, 5, alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Boatfly", "boatfly", alpha); yOffset += 35;
        renderSlider(x, y + yOffset, "Vertical Speed", "boatfly", 0, 5, alpha);
    }
    
    void renderRenderTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        renderToggle(x, y + yOffset, "ESP", "esp", alpha); yOffset += 35;
        // Additional render options would go here
    }
    
    void renderSettingsTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        drawText(x, y + yOffset, "Keybinds", 0.8f, 0.8f, 0.8f); yOffset += 25;
        renderKeybind(x, y + yOffset, "Aimbot", "aimbot", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "KillAura", "killaura", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "ESP", "esp", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Triggerbot", "triggerbot", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Speedwalk", "speedwalk", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Boatfly", "boatfly", alpha);
    }
    
    void renderConfigTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        drawText(x, y + yOffset, "Profile Manager", 0.8f, 0.8f, 0.8f); yOffset += 25;
        
        // Current profile
        drawText(x, y + yOffset, "Current: " + config.getCurrentProfile(), 0.6f, 0.6f, 0.8f); yOffset += 25;
        
        // Buttons
        if (renderButton(x, y + yOffset, "Save Profile", 100, 30, alpha)) {
            config.saveProfile(config.getCurrentProfile());
        }
        if (renderButton(x + 110, y + yOffset, "Load Profile", 100, 30, alpha)) {
            config.loadProfile(config.getCurrentProfile());
        }
        yOffset += 40;
        
        if (renderButton(x, y + yOffset, "New Profile", 100, 30, alpha)) {
            std::string newName = "profile_" + std::to_string(config.getProfiles().size());
            config.saveProfile(newName);
        }
        if (renderButton(x + 110, y + yOffset, "Delete Profile", 100, 30, alpha)) {
            config.deleteProfile(config.getCurrentProfile());
        }
        yOffset += 40;
        
        // List profiles
        drawText(x, y + yOffset, "Profiles:", 0.7f, 0.7f, 0.7f); yOffset += 20;
        for (auto& profile : config.getProfiles()) {
            if (renderButton(x, y + yOffset, profile, 200, 25, alpha)) {
                config.loadProfile(profile);
            }
            yOffset += 30;
        }
    }
    
    // ============================================================
    // UI RENDER HELPERS
    // ============================================================
    void drawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
        // Simulated OpenGL rendering
    }
    
    void drawRectBorder(float x, float y, float w, float h, float r, float g, float b, float a) {
        // Draw border
        drawRect(x, y, w, 1, r, g, b, a);
        drawRect(x, y + h - 1, w, 1, r, g, b, a);
        drawRect(x, y, 1, h, r, g, b, a);
        drawRect(x + w - 1, y, 1, h, r, g, b, a);
    }
    
    void drawText(float x, float y, const std::string& text, float r, float g, float b) {
        // Simulated text rendering
    }
    
    bool renderButton(float x, float y, const std::string& text, float w, float h, float alpha) {
        // Simulated button with hover detection
        drawRect(x, y, w, h, 0.15f, 0.15f, 0.25f, alpha);
        drawRectBorder(x, y, w, h, 0.25f, 0.25f, 0.35f, alpha);
        drawText(x + w/2 - text.length() * 3, y + h/2 - 5, text, 0.8f, 0.8f, 0.8f);
        return false; // Would detect click
    }
    
    void renderToggle(float x, float y, const std::string& label, const std::string& feature, float alpha) {
        bool enabled = config.isFeatureEnabled(feature);
        drawText(x, y + 2, label, 0.7f, 0.7f, 0.7f);
        
        // Toggle switch
        float toggleX = x + 180;
        float toggleWidth = 40;
        float toggleHeight = 20;
        drawRect(toggleX, y, toggleWidth, toggleHeight, 
                enabled ? 0.1f : 0.2f, 
                enabled ? 0.4f : 0.1f, 
                enabled ? 0.1f : 0.1f, 
                alpha);
        drawRect(toggleX + (enabled ? toggleWidth - toggleHeight : 0), y, 
                toggleHeight, toggleHeight, 
                0.9f, 0.9f, 0.9f, alpha);
        
        // Click would toggle config
    }
    
    void renderSlider(float x, float y, const std::string& label, const std::string& feature, 
                     float minVal, float maxVal, float alpha) {
        float value = config.getFeatureValue(feature);
        float normalized = (value - minVal) / (maxVal - minVal);
        
        drawText(x, y + 2, label + ": " + std::to_string((int)value), 0.7f, 0.7f, 0.7f);
        
        float sliderX = x + 180;
        float sliderWidth = 150;
        float sliderHeight = 6;
        float sliderY = y + 10;
        
        // Background
        drawRect(sliderX, sliderY, sliderWidth, sliderHeight, 0.2f, 0.2f, 0.2f, alpha);
        // Fill
        drawRect(sliderX, sliderY, sliderWidth * normalized, sliderHeight, 0.3f, 0.6f, 0.8f, alpha);
        // Handle
        drawRect(sliderX + sliderWidth * normalized - 4, sliderY - 4, 8, 14, 0.8f, 0.8f, 0.8f, alpha);
    }
    
    void renderKeybind(float x, float y, const std::string& label, const std::string& feature, float alpha) {
        int key = config.getKeybind(feature);
        std::string keyName = (key == -1) ? "None" : "Key " + std::to_string(key);
        
        drawText(x, y + 2, label, 0.7f, 0.7f, 0.7f);
        drawRect(x + 180, y, 80, 20, 0.15f, 0.15f, 0.25f, alpha);
        drawText(x + 190, y + 4, keyName, 0.8f, 0.8f, 0.8f);
    }
    
    // ============================================================
    // INPUT HANDLING
    // ============================================================
    void handleClick(int x, int y, bool clicked) {
        // Process UI clicks
        if (!isOpen) return;
        
        // Check if click is on close button
        // Check if click is on tabs
        // Check if click is on toggles, sliders, buttons
    }
    
    void handleKey(int key) {
        if (!isOpen) return;
        
        // Handle keybind picking
        // Handle ESC to close
        if (key == 27) { // ESC
            toggle();
        }
    }
};

// ============================================================
// SECTION 7: MAIN CLIENT CONTROLLER
// ============================================================

class ClientController {
private:
    ConfigManager config;
    EventBus eventBus;
    Minecraft::GameState gameState;
    Minecraft::RenderContext renderContext;
    CheatFeatures* features;
    ClickGUI* gui;
    
    std::atomic<bool> running{true};
    std::thread gameLoop;
    
public:
    ClientController() {
        // Initialize components
        features = new CheatFeatures(config, eventBus, gameState, renderContext);
        gui = new ClickGUI(config, renderContext);
        
        // Start the game loop
        gameLoop = std::thread(&ClientController::runGameLoop, this);
    }
    
    ~ClientController() {
        running = false;
        if (gameLoop.joinable()) {
            gameLoop.join();
        }
        delete features;
        delete gui;
    }
    
    void runGameLoop() {
        while (running) {
            auto start = std::chrono::high_resolution_clock::now();
            
            // Simulate game tick
            gameState.tickCounter++;
            
            // Simulate FPS
            renderContext.deltaTime = 0.016f;
            
            // Publish events
            eventBus.publish(EventBus::TICK);
            eventBus.publish(EventBus::RENDER);
            
            // Frame timing
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (duration.count() < 16) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16 - duration.count()));
            }
        }
    }
    
    void toggleGUI() {
        gui->toggle();
    }
    
    void renderGUI() {
        gui->render();
    }
    
    void handleInput() {
        // Process keyboard/mouse input
        if (gui->isOpen) {
            // Route input to GUI
        }
    }
    
    // Inject game state updates (called from hooked functions)
    void updateGameState(const Minecraft::GameState& state) {
        gameState = state;
    }
};

// ============================================================
// SECTION 8: HOOK IMPLEMENTATION (Simulated)
// ============================================================

// Global client instance
static ClientController* clientInstance = nullptr;

// ============================================================
// HOOK: Render Hook
// ============================================================
void HOOK_Render() {
    if (clientInstance) {
        // Render features
        EventBus::instance().publish(EventBus::RENDER);
        // Render GUI on top
        clientInstance->renderGUI();
    }
}

// ============================================================
// HOOK: Tick Hook
// ============================================================
void HOOK_Tick() {
    if (clientInstance) {
        EventBus::instance().publish(EventBus::TICK);
    }
}

// ============================================================
// HOOK: Key Input Hook
// ============================================================
void HOOK_KeyInput(int key, bool pressed) {
    if (clientInstance) {
        if (pressed && key == 34) { // INS key (example toggle)
            clientInstance->toggleGUI();
        }
        EventBus::instance().publish(EventBus::KEY_INPUT);
        clientInstance->handleInput();
    }
}

// ============================================================
// ENTRY POINT
// ============================================================

extern "C" {
    // DLL/SO Entry Point
    void __attribute__((constructor)) init() {
        if (!clientInstance) {
            clientInstance = new ClientController();
            
            // Register hooks (in production, would use actual hooking)
            // For simulation, we just run the client
            std::cout << "Client initialized successfully!" << std::endl;
            std::cout << "Press INS key to toggle GUI" << std::endl;
        }
    }
    
    void __attribute__((destructor)) cleanup() {
        if (clientInstance) {
            delete clientInstance;
            clientInstance = nullptr;
        }
    }
}

// ============================================================
// SIMULATED MAIN (For testing only)
// ============================================================

#ifdef _SIMULATED_MAIN
#include <conio.h>

int main() {
    init();
    
    // Simulate game loop
    std::cout << "Running client simulation..." << std::endl;
    std::cout << "Press 'q' to quit" << std::endl;
    
    while (true) {
        if (_kbhit()) {
            int key = _getch();
            if (key == 'q') break;
            if (key == 34) { // INS key
                clientInstance->toggleGUI();
            }
        }
        
        // Simulate tick
        EventBus::instance().publish(EventBus::TICK);
        EventBus::instance().publish(EventBus::RENDER);
        clientInstance->renderGUI();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    cleanup();
    return 0;
}
#endif

class RenderPipeline {
private:
    // OpenGL function pointers (simulated)
    struct GLContext {
        void* deviceContext;
        void* renderContext;
    } gl;
    
    // Vertex buffer objects for performance
    struct VBOCache {
        std::vector<float> vertices;
        std::vector<float> colors;
        std::vector<unsigned int> indices;
    } vboCache;
    
    bool glInitialized = false;
    
public:
    void initOpenGL() {
        // In production, would hook wglCreateContext, etc.
        glInitialized = true;
        
        // Setup orthographic projection for 2D rendering
        setupOrtho();
        
        // Initialize VBOs
        glGenBuffers(1, &vboCache.vertices);
        glGenBuffers(1, &vboCache.colors);
        glGenBuffers(1, &vboCache.indices);
    }
    
    void setupOrtho() {
        // Set up 2D projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1920, 1080, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    
    void beginDraw() {
        if (!glInitialized) return;
        
        // Save OpenGL state
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Set line width
        glLineWidth(1.0f);
    }
    
    void endDraw() {
        if (!glInitialized) return;
        // Restore OpenGL state
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    
    // ============================================================
    // PRIMITIVE DRAWING FUNCTIONS
    // ============================================================
    
    void drawFilledRect(float x, float y, float w, float h, float r, float g, float b, float a) {
        if (!glInitialized) return;
        
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    void drawRectBorder(float x, float y, float w, float h, float r, float g, float b, float a) {
        drawFilledRect(x, y, w, 1, r, g, b, a); // Top
        drawFilledRect(x, y + h - 1, w, 1, r, g, b, a); // Bottom
        drawFilledRect(x, y, 1, h, r, g, b, a); // Left
        drawFilledRect(x + w - 1, y, 1, h, r, g, b, a); // Right
    }
    
    void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
        if (!glInitialized) return;
        
        glColor4f(r, g, b, a);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
    
    void drawOutlineRect(float x, float y, float w, float h, float r, float g, float b, float a) {
        drawFilledRect(x, y, w, 1, r, g, b, a);
        drawFilledRect(x, y + h - 1, w, 1, r, g, b, a);
        drawFilledRect(x, y, 1, h, r, g, b, a);
        drawFilledRect(x + w - 1, y, 1, h, r, g, b, a);
    }
    
    void drawGradientRect(float x, float y, float w, float h, 
                          float r1, float g1, float b1, float a1,
                          float r2, float g2, float b2, float a2) {
        if (!glInitialized) return;
        
        glBegin(GL_QUADS);
        glColor4f(r1, g1, b1, a1);
        glVertex2f(x, y);
        glColor4f(r2, g2, b2, a2);
        glVertex2f(x + w, y);
        glColor4f(r2, g2, b2, a2);
        glVertex2f(x + w, y + h);
        glColor4f(r1, g1, b1, a1);
        glVertex2f(x, y + h);
        glEnd();
    }
    
    void drawCircle(float cx, float cy, float radius, float r, float g, float b, float a, int segments = 32) {
        if (!glInitialized) return;
        
        glColor4f(r, g, b, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * 3.14159f * i / segments;
            float x = cx + radius * cos(theta);
            float y = cy + radius * sin(theta);
            glVertex2f(x, y);
        }
        glEnd();
    }
    
    void drawText(const std::string& text, float x, float y, float r, float g, float b, float a = 1.0f) {
        if (!glInitialized || text.empty()) return;
        
        // In production, would use FreeType or MC's font renderer
        // This is a placeholder - actual implementation would call MC's renderText
        // or use a custom bitmap font
        glColor4f(r, g, b, a);
        glRasterPos2f(x, y);
        for (char c : text) {
            // Would render actual glyphs here
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }
    }
    
    void drawTextWithShadow(const std::string& text, float x, float y, float r, float g, float b, float a = 1.0f) {
        drawText(text, x + 1, y + 1, 0, 0, 0, 0.5f);
        drawText(text, x, y, r, g, b, a);
    }
    
    // ============================================================
    // 3D TO 2D PROJECTION
    // ============================================================
    
    bool worldToScreen(float x, float y, float z, float& screenX, float& screenY) {
        // Would use MC's projection matrix
        // This is a simplified version
        float viewMatrix[16], projectionMatrix[16];
        int viewport[4];
        
        // Get matrices from MC
        glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
        glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        double winX, winY, winZ;
        int result = gluProject(x, y, z, viewMatrix, projectionMatrix, viewport, &winX, &winY, &winZ);
        
        if (result == GL_TRUE && winZ < 1.0f) {
            screenX = (float)winX;
            screenY = (float)viewport[3] - (float)winY;
            return true;
        }
        return false;
    }
};

// ============================================================
// SECTION 10: COMPLETE MEMORY READING SYSTEM
// ============================================================

class MemoryReader {
private:
    void* processHandle = nullptr;
    uintptr_t baseAddress = 0;
    uintptr_t clientStatePtr = 0;
    
    struct MemoryOffsets {
        uintptr_t localPlayer = 0x00A3B0A0;
        uintptr_t entityList = 0x00A3B0A8;
        uintptr_t viewMatrix = 0x00A3B0B0;
        uintptr_t gameState = 0x00A3B0B8;
        uintptr_t forceJump = 0x00A3B0C0;
        uintptr_t forceAttack = 0x00A3B0C8;
    } offsets;
    
    // Entity offsets
    struct EntityOffsets {
        uintptr_t health = 0x100;
        uintptr_t position = 0x34;
        uintptr_t viewAngles = 0x40;
        uintptr_t flags = 0x104;
        uintptr_t name = 0x205;
        uintptr_t team = 0x30C;
        uintptr_t dormancy = 0xE0;
        uintptr_t glowIndex = 0x428;
        uintptr_t spotted = 0x93C;
    } entityOffsets;
    
public:
    bool initialize() {
        #ifdef _WIN32
        // Windows: Get process handle
        HWND mcWindow = FindWindowA(NULL, "Minecraft");
        if (!mcWindow) return false;
        
        DWORD processId;
        GetWindowThreadProcessId(mcWindow, &processId);
        processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        
        // Get base address
        HMODULE modules[1024];
        DWORD needed;
        if (EnumProcessModules(processHandle, modules, sizeof(modules), &needed)) {
            baseAddress = (uintptr_t)modules[0];
        }
        #else
        // Linux: Use /proc/pid/maps or similar
        #endif
        
        return processHandle != nullptr;
    }
    
    template<typename T>
    T readMemory(uintptr_t address) {
        T value = {};
        #ifdef _WIN32
        ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(T), nullptr);
        #else
        // Linux memory reading using ptrace or /proc
        #endif
        return value;
    }
    
    template<typename T>
    void writeMemory(uintptr_t address, T value) {
        #ifdef _WIN32
        WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(T), nullptr);
        #else
        // Linux memory writing
        #endif
    }
    
    uintptr_t getModuleBase(const std::string& moduleName) {
        #ifdef _WIN32
        HMODULE modules[1024];
        DWORD needed;
        if (EnumProcessModules(processHandle, modules, sizeof(modules), &needed)) {
            for (int i = 0; i < needed / sizeof(HMODULE); i++) {
                char name[256];
                GetModuleBaseNameA(processHandle, modules[i], name, sizeof(name));
                if (moduleName == name) {
                    return (uintptr_t)modules[i];
                }
            }
        }
        #endif
        return 0;
    }
    
    Minecraft::GameState readGameState() {
        Minecraft::GameState state;
        
        // Read local player
        uintptr_t localPlayerPtr = readMemory<uintptr_t>(baseAddress + offsets.localPlayer);
        if (localPlayerPtr) {
            state.localPlayer.x = readMemory<float>(localPlayerPtr + entityOffsets.position);
            state.localPlayer.y = readMemory<float>(localPlayerPtr + entityOffsets.position + 4);
            state.localPlayer.z = readMemory<float>(localPlayerPtr + entityOffsets.position + 8);
            state.localPlayer.health = readMemory<float>(localPlayerPtr + entityOffsets.health);
            state.localPlayer.yaw = readMemory<float>(localPlayerPtr + entityOffsets.viewAngles);
            state.localPlayer.pitch = readMemory<float>(localPlayerPtr + entityOffsets.viewAngles + 4);
            state.localPlayer.onGround = (readMemory<int>(localPlayerPtr + entityOffsets.flags) & 1) != 0;
            
            // Check if in vehicle
            // Would check for vehicle entity pointer
        }
        
        // Read entity list
        uintptr_t entityListPtr = readMemory<uintptr_t>(baseAddress + offsets.entityList);
        if (entityListPtr) {
            int entityCount = readMemory<int>(entityListPtr + 0x8);
            state.entities.clear();
            
            for (int i = 0; i < std::min(entityCount, 64); i++) {
                uintptr_t entityPtr = readMemory<uintptr_t>(entityListPtr + 0x10 + i * 0x8);
                if (entityPtr) {
                    Minecraft::GameState::Entity entity;
                    entity.x = readMemory<float>(entityPtr + entityOffsets.position);
                    entity.y = readMemory<float>(entityPtr + entityOffsets.position + 4);
                    entity.z = readMemory<float>(entityPtr + entityOffsets.position + 8);
                    entity.health = readMemory<float>(entityPtr + entityOffsets.health);
                    entity.visible = !readMemory<bool>(entityPtr + entityOffsets.dormancy);
                    
                    // Read name
                    char nameBuffer[64];
                    readMemory<char>(entityPtr + entityOffsets.name, nameBuffer, sizeof(nameBuffer));
                    entity.name = std::string(nameBuffer);
                    
                    // Calculate distance
                    entity.distance = Utility::Math::distance3D(
                        state.localPlayer.x, state.localPlayer.y, state.localPlayer.z,
                        entity.x, entity.y, entity.z
                    );
                    
                    // Check team/friend
                    entity.isTeam = (readMemory<int>(entityPtr + entityOffsets.team) == 
                                    readMemory<int>(localPlayerPtr + entityOffsets.team));
                    
                    state.entities.push_back(entity);
                }
            }
        }
        
        return state;
    }
    
    void writeViewAngles(float yaw, float pitch) {
        uintptr_t localPlayerPtr = readMemory<uintptr_t>(baseAddress + offsets.localPlayer);
        if (localPlayerPtr) {
            writeMemory<float>(localPlayerPtr + entityOffsets.viewAngles, yaw);
            writeMemory<float>(localPlayerPtr + entityOffsets.viewAngles + 4, pitch);
        }
    }
    
    void forceAttack() {
        writeMemory<int>(baseAddress + offsets.forceAttack, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        writeMemory<int>(baseAddress + offsets.forceAttack, 0);
    }
};

// ============================================================
// SECTION 11: COMPLETE AIMBOT IMPLEMENTATION
// ============================================================

class CompleteAimbot {
private:
    ConfigManager& config;
    MemoryReader& memory;
    RenderPipeline& renderer;
    Utility::RandomGenerator rng;
    
    struct TargetData {
        int index;
        float distance;
        float angle;
        float health;
        float priorityScore;
    };
    
    std::vector<TargetData> targets;
    int currentTarget = -1;
    Utility::Timer smoothTimer;
    
public:
    CompleteAimbot(ConfigManager& cfg, MemoryReader& mem, RenderPipeline& rend)
        : config(cfg), memory(mem), renderer(rend) {}
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("aimbot")) {
            currentTarget = -1;
            return;
        }
        
        float fov = config.getFeatureValue("aimbot");
        float smooth = 0.15f;
        int priorityMode = 0; // 0=distance, 1=health, 2=angle
        
        // Find valid targets
        targets.clear();
        for (int i = 0; i < state.entities.size(); i++) {
            auto& entity = state.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isTeam) continue;
            if (entity.name.find("NPC") != std::string::npos) continue;
            
            // Calculate angles
            float dx = entity.x - state.localPlayer.x;
            float dy = entity.y - state.localPlayer.y;
            float dz = entity.z - state.localPlayer.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            
            float targetYaw = atan2(dz, dx) * 57.2958f;
            float targetPitch = atan2(dy, sqrt(dx*dx + dz*dz)) * 57.2958f;
            
            float yawDiff = Utility::Math::angleDifference(state.localPlayer.yaw, targetYaw);
            float pitchDiff = Utility::Math::angleDifference(state.localPlayer.pitch, targetPitch);
            float angleDist = sqrt(yawDiff*yawDiff + pitchDiff*pitchDiff);
            
            if (angleDist < fov) {
                TargetData data;
                data.index = i;
                data.distance = dist;
                data.angle = angleDist;
                data.health = entity.health;
                
                // Calculate priority score
                switch (priorityMode) {
                    case 0: data.priorityScore = dist; break;
                    case 1: data.priorityScore = entity.health / 20.0f; break;
                    case 2: data.priorityScore = angleDist; break;
                }
                targets.push_back(data);
            }
        }
        
        // Sort targets by priority
        std::sort(targets.begin(), targets.end(), [&](const TargetData& a, const TargetData& b) {
            return a.priorityScore < b.priorityScore;
        });
        
        if (!targets.empty()) {
            currentTarget = targets[0].index;
            
            // Smooth aim
            float currentYaw = state.localPlayer.yaw;
            float currentPitch = state.localPlayer.pitch;
            
            // Calculate target angles
            auto& entity = state.entities[currentTarget];
            float dx = entity.x - state.localPlayer.x;
            float dy = entity.y - state.localPlayer.y;
            float dz = entity.z - state.localPlayer.z;
            float targetYaw = atan2(dz, dx) * 57.2958f;
            float targetPitch = atan2(dy, sqrt(dx*dx + dz*dz)) * 57.2958f;
            
            // Apply smoothing
            float smoothYaw = Utility::Math::lerp(currentYaw, targetYaw, smooth);
            float smoothPitch = Utility::Math::lerp(currentPitch, targetPitch, smooth);
            
            // Write to memory
            memory.writeViewAngles(smoothYaw, smoothPitch);
        } else {
            currentTarget = -1;
        }
    }
    
    void renderFOVCircle(float screenX, float screenY, float fov) {
        if (!config.isFeatureEnabled("aimbot")) return;
        
        // Draw FOV circle
        float radius = fov * 3.0f; // Scale factor
        renderer.drawCircle(screenX, screenY, radius, 0.3f, 0.6f, 1.0f, 0.3f);
        renderer.drawCircle(screenX, screenY, radius, 0.3f, 0.6f, 1.0f, 0.8f);
    }
};

// ============================================================
// SECTION 12: COMPLETE KILLAURA IMPLEMENTATION
// ============================================================

class CompleteKillAura {
private:
    ConfigManager& config;
    MemoryReader& memory;
    Utility::RandomGenerator rng;
    Utility::Timer attackTimer;
    std::vector<int> targetList;
    int currentTarget = -1;
    
public:
    CompleteKillAura(ConfigManager& cfg, MemoryReader& mem)
        : config(cfg), memory(mem) {}
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("killaura")) {
            targetList.clear();
            return;
        }
        
        float range = config.getFeatureValue("killaura");
        bool multiTarget = false;
        int maxTargets = 1;
        bool autoSwitch = true;
        
        // Find targets in range
        targetList.clear();
        for (int i = 0; i < state.entities.size(); i++) {
            auto& entity = state.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isTeam) continue;
            if (entity.distance > range) continue;
            
            targetList.push_back(i);
        }
        
        // Sort by distance
        std::sort(targetList.begin(), targetList.end(), [&](int a, int b) {
            return state.entities[a].distance < state.entities[b].distance;
        });
        
        // Auto weapon switch
        if (autoSwitch && !targetList.empty()) {
            // Would switch to best weapon
            // state.localPlayer.currentWeapon = "diamond_sword";
        }
        
        // Attack logic
        int attacksPerSecond = 8;
        int minDelay = 1000 / attacksPerSecond;
        int maxDelay = minDelay + rng.nextInt(0, 200);
        
        if (attackTimer.hasPassed(minDelay + rng.nextInt(0, 100))) {
            if (!targetList.empty()) {
                if (multiTarget) {
                    // Attack multiple targets
                    int count = 0;
                    for (int idx : targetList) {
                        if (count >= maxTargets) break;
                        attackEntity(idx, state);
                        count++;
                    }
                } else {
                    // Attack closest target
                    attackEntity(targetList[0], state);
                }
                attackTimer.reset();
            }
        }
    }
    
    void attackEntity(int index, Minecraft::GameState& state) {
        if (index < 0 || index >= state.entities.size()) return;
        
        // Simulate attack
        state.entities[index].health -= rng.nextFloat(0.5f, 1.5f);
        if (state.entities[index].health < 0) {
            state.entities[index].health = 0;
        }
        
        // Send attack packet
        memory.forceAttack();
    }
};

// ============================================================
// SECTION 13: COMPLETE ESP IMPLEMENTATION
// ============================================================

class CompleteESP {
private:
    ConfigManager& config;
    RenderPipeline& renderer;
    MemoryReader& memory;
    
    struct ESPData {
        float screenX, screenY;
        float distance;
        float health;
        std::string name;
        bool isVisible;
    };
    
    std::vector<ESPData> espData;
    Utility::Timer updateTimer;
    float screenCenterX, screenCenterY;
    
public:
    CompleteESP(ConfigManager& cfg, RenderPipeline& rend, MemoryReader& mem)
        : config(cfg), renderer(rend), memory(mem) {
        screenCenterX = 960;
        screenCenterY = 540;
    }
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("esp")) {
            espData.clear();
            return;
        }
        
        // Update every 50ms for performance
        if (updateTimer.hasPassed(50)) {
            espData.clear();
            
            for (auto& entity : state.entities) {
                if (entity.health <= 0) continue;
                
                float screenX, screenY;
                if (renderer.worldToScreen(entity.x, entity.y, entity.z, screenX, screenY)) {
                    ESPData data;
                    data.screenX = screenX;
                    data.screenY = screenY;
                    data.distance = entity.distance;
                    data.health = entity.health;
                    data.name = entity.name;
                    data.isVisible = entity.visible;
                    espData.push_back(data);
                }
            }
            
            updateTimer.reset();
        }
    }
    
    void render(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("esp")) return;
        
        for (auto& data : espData) {
            // Distance-based scaling
            float scale = 1.0f / (1.0f + data.distance * 0.05f);
            float boxWidth = 40.0f * scale;
            float boxHeight = 60.0f * scale;
            
            // Color based on health
            float healthRatio = data.health / 20.0f;
            float r = 1.0f - healthRatio;
            float g = healthRatio;
            float b = 0.0f;
            
            // 2D Box
            renderer.drawOutlineRect(
                data.screenX - boxWidth/2,
                data.screenY - boxHeight/2,
                boxWidth, boxHeight,
                r, g, b, 0.8f
            );
            
            // Filled box with transparency
            renderer.drawFilledRect(
                data.screenX - boxWidth/2,
                data.screenY - boxHeight/2,
                boxWidth, boxHeight,
                r, g, b, 0.1f
            );
            
            // Tracer from bottom of screen
            renderer.drawLine(
                screenCenterX, screenCenterY,
                data.screenX, data.screenY - boxHeight/2,
                r, g, b, 0.5f
            );
            
            // Health bar
            float barWidth = boxWidth;
            float barHeight = 3.0f;
            float barX = data.screenX - boxWidth/2;
            float barY = data.screenY + boxHeight/2 + 5;
            
            // Health bar background
            renderer.drawFilledRect(
                barX, barY,
                barWidth, barHeight,
                0.2f, 0.2f, 0.2f, 0.5f
            );
            
            // Health bar fill
            renderer.drawFilledRect(
                barX, barY,
                barWidth * healthRatio, barHeight,
                r, g, b, 0.8f
            );
            
            // Name and distance
            std::string info = data.name + " [" + std::to_string((int)data.distance) + "m]";
            renderer.drawTextWithShadow(
                info,
                data.screenX - 20, data.screenY - boxHeight/2 - 15,
                1.0f, 1.0f, 1.0f, 0.9f
            );
            
            // Health text
            std::string healthText = std::to_string((int)data.health) + " HP";
            renderer.drawTextWithShadow(
                healthText,
                data.screenX - 15, data.screenY + boxHeight/2 + 10,
                1.0f, 1.0f, 1.0f, 0.7f
            );
        }
    }
};

// ============================================================
// SECTION 14: COMPLETE TRIGGERBOT IMPLEMENTATION
// ============================================================

class CompleteTriggerbot {
private:
    ConfigManager& config;
    MemoryReader& memory;
    Utility::Timer delayTimer;
    bool isActive = false;
    int lastTarget = -1;
    
public:
    CompleteTriggerbot(ConfigManager& cfg, MemoryReader& mem)
        : config(cfg), memory(mem) {}
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("triggerbot")) {
            isActive = false;
            return;
        }
        
        bool holdingKey = true; // Would check actual key state
        if (!holdingKey) {
            isActive = false;
            return;
        }
        
        // Check crosshair target
        int target = getCrosshairTarget(state);
        if (target != -1) {
            float delay = config.getFeatureValue("triggerbot");
            if (delayTimer.hasPassed(delay)) {
                // Attack target
                if (state.entities[target].health > 0) {
                    memory.forceAttack();
                    state.entities[target].health -= 1.0f;
                    delayTimer.reset();
                    
                    lastTarget = target;
                    isActive = true;
                }
            }
        } else {
            lastTarget = -1;
            isActive = false;
        }
    }
    
    int getCrosshairTarget(Minecraft::GameState& state) {
        float fov = 1.5f; // Very tight for triggerbot
        int best = -1;
        float bestAngle = fov + 1.0f;
        
        for (int i = 0; i < state.entities.size(); i++) {
            auto& entity = state.entities[i];
            if (entity.health <= 0) continue;
            if (!entity.visible) continue;
            if (entity.isTeam) continue;
            
            float dx = entity.x - state.localPlayer.x;
            float dy = entity.y - state.localPlayer.y;
            float dz = entity.z - state.localPlayer.z;
            
            float targetYaw = atan2(dz, dx) * 57.2958f;
            float targetPitch = atan2(dy, sqrt(dx*dx + dz*dz)) * 57.2958f;
            
            float yawDiff = Utility::Math::angleDifference(state.localPlayer.yaw, targetYaw);
            float pitchDiff = Utility::Math::angleDifference(state.localPlayer.pitch, targetPitch);
            float angleDist = sqrt(yawDiff*yawDiff + pitchDiff*pitchDiff);
            
            if (angleDist < fov && angleDist < bestAngle) {
                bestAngle = angleDist;
                best = i;
            }
        }
        return best;
    }
};

// ============================================================
// SECTION 15: COMPLETE SPEEDWALK IMPLEMENTATION
// ============================================================

class CompleteSpeedwalk {
private:
    ConfigManager& config;
    MemoryReader& memory;
    float currentSpeed = 0.0f;
    
public:
    CompleteSpeedwalk(ConfigManager& cfg, MemoryReader& mem)
        : config(cfg), memory(mem) {}
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("speedwalk")) return;
        
        float multiplier = config.getFeatureValue("speedwalk");
        bool groundOnly = true;
        bool strafeModify = false;
        
        if (groundOnly && !state.localPlayer.onGround) {
            // Reset speed when in air
            currentSpeed = 0.0f;
            return;
        }
        
        // Modify movement speed
        float baseSpeed = 0.1f;
        float targetSpeed = baseSpeed * multiplier;
        
        // Smooth speed transition
        currentSpeed = Utility::Math::lerp(currentSpeed, targetSpeed, 0.2f);
        
        // Apply to game state (would hook movement in production)
        if (state.localPlayer.onGround || !groundOnly) {
            // Modify horizontal velocity
            // This is where we'd write to movement variables
            // For simulation, we just update state
            state.localPlayer.x += currentSpeed * 0.1f;
        }
        
        // Air strafe modification
        if (strafeModify && !state.localPlayer.onGround) {
            // Modify strafe speed in air
            // Would hook movement calculation
        }
    }
};

// ============================================================
// SECTION 16: COMPLETE BOATFLY IMPLEMENTATION
// ============================================================

class CompleteBoatfly {
private:
    ConfigManager& config;
    MemoryReader& memory;
    float verticalVelocity = 0.0f;
    float horizontalVelocity = 0.0f;
    Utility::Timer glideTimer;
    
public:
    CompleteBoatfly(ConfigManager& cfg, MemoryReader& mem)
        : config(cfg), memory(mem) {}
    
    void update(Minecraft::GameState& state) {
        if (!config.isFeatureEnabled("boatfly")) return;
        
        // Check if in vehicle
        if (!state.localPlayer.inVehicle) {
            verticalVelocity = 0.0f;
            horizontalVelocity = 0.0f;
            return;
        }
        
        float verticalSpeed = config.getFeatureValue("boatfly");
        float horizontalSpeed = verticalSpeed * 0.5f;
        bool glideMode = false;
        
        // Check key inputs for flight control
        bool jumpKey = false;
        bool sneakKey = false;
        bool forwardKey = false;
        bool backwardKey = false;
        bool leftKey = false;
        bool rightKey = false;
        
        // Vertical movement
        if (jumpKey) {
            verticalVelocity = verticalSpeed * 0.1f;
        } else if (sneakKey) {
            verticalVelocity = -verticalSpeed * 0.1f;
        } else if (glideMode) {
            // Glide with slight descent
            verticalVelocity = -0.02f;
            if (glideTimer.hasPassed(500)) {
                verticalVelocity -= 0.01f;
                glideTimer.reset();
            }
        } else {
            // Hover in place
            verticalVelocity = 0.0f;
        }
        
        // Horizontal movement
        float hSpeed = horizontalSpeed * 0.1f;
        if (forwardKey) horizontalVelocity = hSpeed;
        else if (backwardKey) horizontalVelocity = -hSpeed;
        else horizontalVelocity = 0.0f;
        
        // Apply to vehicle (would hook vehicle movement)
        // For simulation, we update position
        state.localPlayer.x += horizontalVelocity * 0.1f;
        state.localPlayer.y += verticalVelocity * 0.1f;
        state.localPlayer.z += horizontalVelocity * 0.1f;
    }
};

// ============================================================
// SECTION 17: COMPLETE CLICKGUI IMPLEMENTATION
// ============================================================

class CompleteClickGUI {
private:
    ConfigManager& config;
    RenderPipeline& renderer;
    bool isOpen = false;
    int selectedTab = 0;
    std::vector<std::string> tabs = {"Combat", "Movement", "Render", "Settings", "Config"};
    float animProgress = 0.0f;
    Utility::Timer animTimer;
    Utility::Timer clickCooldown;
    
    // UI State
    struct UIElement {
        std::string name;
        std::string feature;
        float x, y, w, h;
        bool hovered = false;
        bool active = false;
    };
    
    std::vector<UIElement> elements;
    int selectedElement = -1;
    bool pickingKeybind = false;
    std::string pickingFeature = "";
    
public:
    CompleteClickGUI(ConfigManager& cfg, RenderPipeline& rend)
        : config(cfg), renderer(rend) {}
    
    void toggle() {
        isOpen = !isOpen;
        if (isOpen) {
            animProgress = 0.0f;
            animTimer.reset();
            pickingKeybind = false;
        }
    }
    
    void render() {
        if (!isOpen) return;
        
        // Animation
        float animDuration = 300.0f;
        if (!animTimer.hasPassed(animDuration)) {
            animProgress = animTimer.elapsedMillis() / animDuration;
            animProgress = std::min(animProgress, 1.0f);
            // Cubic ease out
            float t = animProgress - 1.0f;
            animProgress = t * t * t + 1.0f;
        } else {
            animProgress = 1.0f;
        }
        
        // Window dimensions
        float winW = 500;
        float winH = 600;
        float winX = (1920 - winW) / 2;
        float winY = (1080 - winH) / 2;
        
        // Scale animation
        float scale = 0.85f + 0.15f * animProgress;
        float alpha = 0.7f + 0.3f * animProgress;
        
        // Window background
        renderer.drawFilledRect(winX, winY, winW, winH, 0.08f, 0.08f, 0.12f, alpha * 0.95f);
        renderer.drawRectBorder(winX, winY, winW, winH, 0.2f, 0.2f, 0.3f, alpha);
        
        // Title bar
        renderer.drawFilledRect(winX, winY, winW, 40, 0.12f, 0.12f, 0.18f, alpha);
        renderer.drawTextWithShadow("Astrix Client v3.0", winX + 20, winY + 12, 0.9f, 0.8f, 1.0f, 1.0f);
        
        // Close button
        float closeX = winX + winW - 35;
        float closeY = winY + 5;
        renderer.drawFilledRect(closeX, closeY, 25, 25, 0.3f, 0.1f, 0.1f, alpha);
        renderer.drawText("X", closeX + 9, closeY + 7, 1.0f, 0.3f, 0.3f, 1.0f);
        
        // Tabs
        float tabY = winY + 45;
        float tabW = (winW - 10) / tabs.size();
        float tabH = 35;
        
        for (int i = 0; i < tabs.size(); i++) {
            float tabX = winX + 5 + i * tabW;
            bool isSelected = (i == selectedTab);
            
            renderer.drawFilledRect(tabX, tabY, tabW - 2, tabH,
                isSelected ? 0.15f : 0.08f,
                isSelected ? 0.08f : 0.06f,
                isSelected ? 0.25f : 0.10f,
                alpha
            );
            
            renderer.drawText(tabs[i], tabX + tabW/2 - 20, tabY + 10,
                isSelected ? 0.8f : 0.5f,
                isSelected ? 0.6f : 0.5f,
                isSelected ? 1.0f : 0.5f,
                1.0f
            );
            
            // Underline for selected tab
            if (isSelected) {
                renderer.drawFilledRect(tabX + 5, tabY + tabH - 2, tabW - 14, 2,
                    0.4f, 0.2f, 0.8f, alpha);
            }
        }
        
        // Content area
        float contentX = winX + 15;
        float contentY = winY + 90;
        float contentW = winW - 30;
        float contentH = winH - 105;
        
        // Render tab content
        switch (selectedTab) {
            case 0: renderCombatTab(contentX, contentY, contentW, contentH, alpha); break;
            case 1: renderMovementTab(contentX, contentY, contentW, contentH, alpha); break;
            case 2: renderRenderTab(contentX, contentY, contentW, contentH, alpha); break;
            case 3: renderSettingsTab(contentX, contentY, contentW, contentH, alpha); break;
            case 4: renderConfigTab(contentX, contentY, contentW, contentH, alpha); break;
        }
    }
    
    void renderCombatTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        
        // Aimbot section
        renderSectionHeader(x, y + yOffset, "Aimbot", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "aimbot", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "FOV", "aimbot", 0, 90, "%d", alpha); yOffset += 40;
        renderSlider(x, y + yOffset, "Smoothness", "aimbot", 0, 100, "%d%%", alpha); yOffset += 40;
        renderDropdown(x, y + yOffset, "Priority", "aimbot", {"Distance", "Health", "Angle"}, alpha); yOffset += 40;
        
        // KillAura section
        renderSectionHeader(x, y + yOffset, "KillAura", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "killaura", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "Range", "killaura", 3, 6, "%.1f", alpha); yOffset += 40;
        renderSlider(x, y + yOffset, "APS", "killaura", 4, 20, "%d", alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Multi-Target", "killaura_multi", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Auto Weapon", "killaura_autoswitch", alpha); yOffset += 30;
        
        // Triggerbot section
        renderSectionHeader(x, y + yOffset, "Triggerbot", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "triggerbot", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "Delay (ms)", "triggerbot", 0, 500, "%d", alpha);
    }
    
    void renderMovementTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        
        renderSectionHeader(x, y + yOffset, "Speedwalk", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "speedwalk", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "Multiplier", "speedwalk", 1, 5, "%.1fx", alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Ground Only", "speedwalk_ground", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Strafe Modify", "speedwalk_strafe", alpha); yOffset += 40;
        
        renderSectionHeader(x, y + yOffset, "Boatfly", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "boatfly", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "Vertical Speed", "boatfly", 0, 5, "%.1f", alpha); yOffset += 40;
        renderSlider(x, y + yOffset, "Horizontal Speed", "boatfly_h", 0, 3, "%.1f", alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Glide Mode", "boatfly_glide", alpha);
    }
    
    void renderRenderTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        
        renderSectionHeader(x, y + yOffset, "ESP", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Enable", "esp", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Boxes", "esp_boxes", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Tracers", "esp_tracers", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Health Bars", "esp_health", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Names", "esp_names", alpha); yOffset += 30;
        renderToggle(x, y + yOffset, "Distance", "esp_distance", alpha); yOffset += 40;
        
        renderSectionHeader(x, y + yOffset, "Colors", alpha); yOffset += 30;
        renderColorPicker(x, y + yOffset, "Box Color", "esp_box_color", alpha); yOffset += 30;
        renderColorPicker(x, y + yOffset, "Tracer Color", "esp_tracer_color", alpha);
    }
    
    void renderSettingsTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        
        renderSectionHeader(x, y + yOffset, "Keybinds", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Toggle GUI", "gui_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Aimbot", "aimbot_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "KillAura", "killaura_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "ESP", "esp_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Triggerbot", "triggerbot_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Speedwalk", "speedwalk_key", alpha); yOffset += 30;
        renderKeybind(x, y + yOffset, "Boatfly", "boatfly_key", alpha); yOffset += 40;
        
        renderSectionHeader(x, y + yOffset, "Visual Settings", alpha); yOffset += 30;
        renderSlider(x, y + yOffset, "GUI Scale", "gui_scale", 50, 150, "%d%%", alpha); yOffset += 40;
        renderToggle(x, y + yOffset, "Show FPS", "show_fps", alpha);
    }
    
    void renderConfigTab(float x, float y, float w, float h, float alpha) {
        float yOffset = 0;
        
        renderSectionHeader(x, y + yOffset, "Profile Manager", alpha); yOffset += 30;
        
        // Current profile
        renderer.drawText("Current: " + config.getCurrentProfile(), x + 5, y + yOffset, 
            0.6f, 0.6f, 0.8f, 1.0f);
        yOffset += 25;
        
        // Action buttons
        float btnW = 100;
        float btnH = 30;
        
        if (renderButton(x, y + yOffset, "Save", btnW, btnH, alpha)) {
            config.saveProfile(config.getCurrentProfile());
        }
        if (renderButton(x + btnW + 5, y + yOffset, "Load", btnW, btnH, alpha)) {
            config.loadProfile(config.getCurrentProfile());
        }
        if (renderButton(x + (btnW + 5) * 2, y + yOffset, "New", btnW, btnH, alpha)) {
            std::string newName = "profile_" + std::to_string(config.getProfiles().size() + 1);
            config.saveProfile(newName);
        }
        yOffset += 40;
        
        if (renderButton(x, y + yOffset, "Delete", btnW, btnH, alpha)) {
            config.deleteProfile(config.getCurrentProfile());
        }
        if (renderButton(x + btnW + 5, y + yOffset, "Rename", btnW, btnH, alpha)) {
            // Would open rename dialog
        }
        if (renderButton(x + (btnW + 5) * 2, y + yOffset, "Export", btnW, btnH, alpha)) {
            config.saveToFile(config.getCurrentProfile());
        }
        yOffset += 40;
        
        // Profile list
        renderSectionHeader(x, y + yOffset, "Profiles", alpha); yOffset += 30;
        for (auto& profile : config.getProfiles()) {
            if (renderButton(x + 5, y + yOffset, profile, w - 10, 25, alpha)) {
                config.loadProfile(profile);
            }
            yOffset += 30;
        }
    }
    
    // ============================================================
    // UI ELEMENT RENDER HELPERS
    // ============================================================
    
    void renderSectionHeader(float x, float y, const std::string& text, float alpha) {
        renderer.drawFilledRect(x, y + 5, 200, 1, 0.3f, 0.3f, 0.5f, alpha * 0.5f);
        renderer.drawText(text, x + 210, y, 0.7f, 0.7f, 0.9f, 1.0f);
    }
    
    bool renderButton(float x, float y, const std::string& text, float w, float h, float alpha) {
        bool hover = isMouseInRect(x, y, w, h);
        
        renderer.drawFilledRect(x, y, w, h, 
            hover ? 0.2f : 0.12f,
            hover ? 0.12f : 0.08f,
            hover ? 0.35f : 0.15f,
            alpha
        );
        renderer.drawRectBorder(x, y, w, h,
            hover ? 0.3f : 0.15f,
            hover ? 0.2f : 0.15f,
            hover ? 0.5f : 0.25f,
            alpha
        );
        
        renderer.drawText(text, x + w/2 - text.length() * 3, y + h/2 - 6,
            0.9f, 0.9f, 0.9f, 1.0f
        );
        
        return hover && wasClicked();
    }
    
    void renderToggle(float x, float y, const std::string& label, const std::string& feature, float alpha) {
        bool enabled = config.isFeatureEnabled(feature);
        
        renderer.drawText(label, x, y + 3, 0.8f, 0.8f, 0.8f, 1.0f);
        
        float toggleX = x + 160;
        float toggleW = 40;
        float toggleH = 20;
        
        renderer.drawFilledRect(toggleX, y, toggleW, toggleH,
            enabled ? 0.2f : 0.15f,
            enabled ? 0.5f : 0.1f,
            enabled ? 0.2f : 0.1f,
            alpha
        );
        renderer.drawRectBorder(toggleX, y, toggleW, toggleH, 0.2f, 0.2f, 0.3f, alpha);
        
        float handleX = enabled ? toggleX + toggleW - toggleH : toggleX;
        renderer.drawFilledRect(handleX, y, toggleH, toggleH,
            0.9f, 0.9f, 0.9f, 1.0f
        );
        renderer.drawRectBorder(handleX, y, toggleH, toggleH, 0.2f, 0.2f, 0.2f, alpha);
        
        // Click detection
        if (isMouseInRect(toggleX, y, toggleW, toggleH) && wasClicked()) {
            config.setFeatureEnabled(feature, !enabled);
        }
    }
    
    void renderSlider(float x, float y, const std::string& label, const std::string& feature,
                     float minVal, float maxVal, const std::string& format, float alpha) {
        float value = config.getFeatureValue(feature);
        float normalized = (value - minVal) / (maxVal - minVal);
        
        char text[64];
        sprintf(text, format.c_str(), value);
        renderer.drawText(label + ": " + text, x, y + 3, 0.8f, 0.8f, 0.8f, 1.0f);
        
        float sliderX = x + 180;
        float sliderW = 200;
        float sliderH = 6;
        float sliderY = y + 12;
        
        // Background
        renderer.drawFilledRect(sliderX, sliderY, sliderW, sliderH,
            0.15f, 0.15f, 0.2f, alpha
        );
        
        // Fill
        renderer.drawFilledRect(sliderX, sliderY, sliderW * normalized, sliderH,
            0.3f, 0.6f, 0.9f, alpha
        );
        
        // Handle
        float handleX = sliderX + sliderW * normalized - 6;
        renderer.drawFilledRect(handleX, sliderY - 4, 12, 14,
            0.9f, 0.9f, 0.9f, 1.0f
        );
        renderer.drawRectBorder(handleX, sliderY - 4, 12, 14, 0.2f, 0.2f, 0.2f, alpha);
        
        // Value display
        char valStr[16];
        sprintf(valStr, format.c_str(), value);
        renderer.drawText(valStr, sliderX + sliderW + 10, y + 3,
            0.8f, 0.8f, 0.8f, 1.0f
        );
        
        // Slider interaction
        if (isMouseInRect(sliderX, sliderY - 4, sliderW, 14) && wasClicked()) {
            // Start dragging
        }
        if (isDragging() && isMouseInRect(sliderX, sliderY - 4, sliderW, 14)) {
            float mouseX = getMouseX();
            float newNorm = (mouseX - sliderX) / sliderW;
            newNorm = std::max(0.0f, std::min(1.0f, newNorm));
            config.setFeatureValue(feature, minVal + newNorm * (maxVal - minVal));
        }
    }
    
    void renderDropdown(float x, float y, const std::string& label, const std::string& feature,
                       const std::vector<std::string>& options, float alpha) {
        renderer.drawText(label, x, y + 3, 0.8f, 0.8f, 0.8f, 1.0f);
        
        float ddX = x + 160;
        float ddW = 120;
        float ddH = 22;
        
        renderer.drawFilledRect(ddX, y, ddW, ddH, 0.12f, 0.12f, 0.18f, alpha);
        renderer.drawRectBorder(ddX, y, ddW, ddH, 0.2f, 0.2f, 0.3f, alpha);
        
        std::string current = options[0]; // Would get from config
        renderer.drawText(current, ddX + 5, y + 5, 0.8f, 0.8f, 0.8f, 1.0f);
        
        // Dropdown arrow
        renderer.drawText("▼", ddX + ddW - 20, y + 2, 0.5f, 0.5f, 0.5f, 1.0f);
    }
    
    void renderColorPicker(float x, float y, const std::string& label, const std::string& feature, float alpha) {
        renderer.drawText(label, x, y + 3, 0.8f, 0.8f, 0.8f, 1.0f);
        
        float pickerX = x + 160;
        float pickerW = 50;
        float pickerH = 20;
        
        // Would use stored color values
        renderer.drawFilledRect(pickerX, y, pickerW, pickerH, 0.8f, 0.2f, 0.2f, alpha);
        renderer.drawRectBorder(pickerX, y, pickerW, pickerH, 0.2f, 0.2f, 0.3f, alpha);
    }
    
    void renderKeybind(float x, float y, const std::string& label, const std::string& feature, float alpha) {
        int key = config.getKeybind(feature);
        std::string keyName = (key == -1) ? "None" : getKeyName(key);
        
        renderer.drawText(label, x, y + 3, 0.8f, 0.8f, 0.8f, 1.0f);
        
        float kbX = x + 160;
        float kbW = 100;
        float kbH = 22;
        
        bool isPicking = (pickingKeybind && pickingFeature == feature);
        renderer.drawFilledRect(kbX, y, kbW, kbH,
            isPicking ? 0.25f : 0.12f,
            isPicking ? 0.1f : 0.08f,
            isPicking ? 0.3f : 0.15f,
            alpha
        );
        renderer.drawRectBorder(kbX, y, kbW, kbH, 0.2f, 0.2f, 0.3f, alpha);
        
        renderer.drawText(isPicking ? "Press key..." : keyName, kbX + 5, y + 4,
            isPicking ? 0.9f : 0.7f,
            isPicking ? 0.7f : 0.7f,
            isPicking ? 1.0f : 0.7f,
            1.0f
        );
        
        if (isMouseInRect(kbX, y, kbW, kbH) && wasClicked()) {
            if (pickingKeybind && pickingFeature == feature) {
                pickingKeybind = false;
            } else {
                pickingKeybind = true;
                pickingFeature = feature;
            }
        }
    }
    
    // ============================================================
    // UTILITY FUNCTIONS
    // ============================================================
    
    std::string getKeyName(int key) {
        // Would map key codes to names
        switch (key) {
            case 0x1B: return "ESC";
            case 0x2D: return "INS";
            case 0x24: return "HOME";
            case 0x21: return "PGUP";
            case 0x22: return "PGDN";
            case 0x2E: return "DEL";
            default: return std::to_string(key);
        }
    }
    
    bool isMouseInRect(float x, float y, float w, float h) {
        float mx = getMouseX();
        float my = getMouseY();
        return mx >= x && mx <= x + w && my >= y && my <= y + h;
    }
    
    float getMouseX() {
        // Would get actual mouse position
        return 0;
    }
    
    float getMouseY() {
        return 0;
    }
    
    bool wasClicked() {
        // Would detect click event
        return false;
    }
    
    bool isDragging() {
        // Would detect drag state
        return false;
    }
    
    void handleClick(int x, int y) {
        if (!isOpen) return;
        
        // Check close button
        // Check tabs
        // Check elements
    }
    
    void handleKey(int key) {
        if (!isOpen) return;
        
        if (pickingKeybind) {
            config.setKeybind(pickingFeature, key);
            pickingKeybind = false;
        }
        
        if (key == 0x1B) { // ESC
            toggle();
        }
    }
};

// ============================================================
// SECTION 18: MAIN CLIENT - FINAL INTEGRATION
// ============================================================

class AstravaClient {
private:
    ConfigManager config;
    MemoryReader memory;
    RenderPipeline renderer;
    Minecraft::GameState gameState;
    
    CompleteAimbot* aimbot;
    CompleteKillAura* killaura;
    CompleteESP* esp;
    CompleteTriggerbot* triggerbot;
    CompleteSpeedwalk* speedwalk;
    CompleteBoatfly* boatfly;
    CompleteClickGUI* gui;
    
    Utility::Timer frameTimer;
    Utility::Timer updateTimer;
    bool running = true;
    std::thread mainThread;
    
public:
    Astra() {
        // Initialize components
        if (!memory.initialize()) {
            std::cout << "Failed to initialize memory reader!" << std::endl;
        }
        
        renderer.initOpenGL();
        
        // Create features
        aimbot = new CompleteAimbot(config, memory, renderer);
        killaura = new CompleteKillAura(config, memory);
        esp = new CompleteESP(config, renderer, memory);
        triggerbot = new CompleteTriggerbot(config, memory);
        speedwalk = new CompleteSpeedwalk(config, memory);
        boatfly = new CompleteBoatfly(config, memory);
        gui = new CompleteClickGUI(config, renderer);
        
        // Start main loop
                mainThread = std::thread(&AstrixClient::run, this);
    }
    
    ~AstrixClient() {
        running = false;
        if (mainThread.joinable()) {
            mainThread.join();
        }
        delete aimbot;
        delete killaura;
        delete esp;
        delete triggerbot;
        delete speedwalk;
        delete boatfly;
        delete gui;
    }
    
    void run() {
        while (running) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            // Read game state
            gameState = memory.readGameState();
            
            // Update features
            aimbot->update(gameState);
            killaura->update(gameState);
            esp->update(gameState);
            triggerbot->update(gameState);
            speedwalk->update(gameState);
            boatfly->update(gameState);
            
            // Render
            renderer.beginDraw();
            
            // Render ESP
            esp->render(gameState);
            
            // Render FOV circle
            if (config.isFeatureEnabled("aimbot")) {
                aimbot->renderFOVCircle(960, 540, config.getFeatureValue("aimbot"));
            }
            
            // Render GUI
            gui->render();
            
            // Render FPS counter
            if (config.isFeatureEnabled("show_fps")) {
                renderer.drawTextWithShadow("FPS: 60", 10, 10, 0.0f, 1.0f, 0.0f, 1.0f);
            }
            
            renderer.endDraw();
            
            // Frame rate limiting
            auto frameEnd = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
            if (duration.count() < 16) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16 - duration.count()));
            }
        }
    }
    
    // ============================================================
    // HOOK CALLBACKS (Called from hooked functions)
    // ============================================================
    
    void onRender() {
        // Hooked from MC render function
        renderer.beginDraw();
        esp->render(gameState);
        gui->render();
        renderer.endDraw();
    }
    
    void onTick() {
        // Hooked from MC tick function
        gameState = memory.readGameState();
        aimbot->update(gameState);
        killaura->update(gameState);
        triggerbot->update(gameState);
        speedwalk->update(gameState);
        boatfly->update(gameState);
    }
    
    void onKeyPress(int key) {
        if (key == config.getKeybind("gui_key")) {
            gui->toggle();
        }
        
        // Toggle features by keybind
        if (key == config.getKeybind("aimbot_key")) {
            config.setFeatureEnabled("aimbot", !config.isFeatureEnabled("aimbot"));
        }
        if (key == config.getKeybind("killaura_key")) {
            config.setFeatureEnabled("killaura", !config.isFeatureEnabled("killaura"));
        }
        if (key == config.getKeybind("esp_key")) {
            config.setFeatureEnabled("esp", !config.isFeatureEnabled("esp"));
        }
        if (key == config.getKeybind("triggerbot_key")) {
            config.setFeatureEnabled("triggerbot", !config.isFeatureEnabled("triggerbot"));
        }
        if (key == config.getKeybind("speedwalk_key")) {
            config.setFeatureEnabled("speedwalk", !config.isFeatureEnabled("speedwalk"));
        }
        if (key == config.getKeybind("boatfly_key")) {
            config.setFeatureEnabled("boatfly", !config.isFeatureEnabled("boatfly"));
        }
        
        gui->handleKey(key);
    }
    
    void onMouseClick(int x, int y, int button) {
        gui->handleClick(x, y);
    }
    
    // ============================================================
    // CONFIGURATION HELPERS
    // ============================================================
    
    void loadConfig(const std::string& profile) {
        config.loadProfile(profile);
    }
    
    void saveConfig(const std::string& profile) {
        config.saveProfile(profile);
    }
};

// ============================================================
// SECTION 19: HOOK MANAGER
// ============================================================

class HookManager {
private:
    static AstrixClient* client;
    
    // Hook function pointers
    typedef void (*RenderHook)();
    typedef void (*TickHook)();
    typedef void (*KeyHook)(int, bool);
    typedef void (*MouseHook)(int, int, int);
    
    // Original function pointers
    RenderHook originalRender = nullptr;
    TickHook originalTick = nullptr;
    KeyHook originalKey = nullptr;
    MouseHook originalMouse = nullptr;
    
public:
    static void initialize(AstrixClient* cl) {
        client = cl;
        
        // Would hook functions here
        // For Windows: using MinHook or Detours
        // For Linux: using LD_PRELOAD or function detouring
    }
    
    // ============================================================
    // HOOK FUNCTIONS (Called by hooked code)
    // ============================================================
    
    static void HOOK_Render() {
        if (client) {
            client->onRender();
        }
        // Call original
        // originalRender();
    }
    
    static void HOOK_Tick() {
        if (client) {
            client->onTick();
        }
        // Call original
        // originalTick();
    }
    
    static void HOOK_KeyInput(int key, bool pressed) {
        if (client && pressed) {
            client->onKeyPress(key);
        }
        // Call original
        // originalKey(key, pressed);
    }
    
    static void HOOK_MouseClick(int x, int y, int button) {
        if (client) {
            client->onMouseClick(x, y, button);
        }
        // Call original
        // originalMouse(x, y, button);
    }
};

// Initialize static member
AstrixClient* HookManager::client = nullptr;

// ============================================================
// SECTION 20: DLL/SO ENTRY POINT
// ============================================================

#ifdef _WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // Create console for debugging
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
            freopen("CONIN$", "r", stdin);
            
            // Initialize client
            AstrixClient* client = new AstrixClient();
            HookManager::initialize(client);
            
            // Install hooks
            // MH_Initialize();
            // MH_CreateHook(&SomeFunction, &HookManager::HOOK_Render, (void**)&originalRender);
            // MH_EnableHook(MH_ALL_HOOKS);
            
            std::cout << "Astrix Client loaded successfully!" << std::endl;
            std::cout << "Press INS key to open GUI" << std::endl;
            break;
            
        case DLL_PROCESS_DETACH:
            // Cleanup
            // MH_DisableHook(MH_ALL_HOOKS);
            // MH_Uninitialize();
            break;
    }
    return TRUE;
}

#else // Linux

__attribute__((constructor)) void init() {
    AstrixClient* client = new AstrixClient();
    HookManager::initialize(client);
    
    // Install hooks using LD_PRELOAD techniques
    std::cout << "Astrix Client loaded successfully!" << std::endl;
    std::cout << "Press INS key to open GUI" << std::endl;
}

__attribute__((destructor)) void cleanup() {
    // Cleanup
}

#endif

// ============================================================
// SECTION 21: TESTING / SIMULATION MAIN
// ============================================================

#ifdef _SIMULATED_MAIN

#include <conio.h>
#include <windows.h>

class Simulator {
private:
    AstrixClient* client;
    bool running = true;
    
public:
    Simulator() {
        client = new AstrixClient();
        HookManager::initialize(client);
        
        // Simulate game loop
        std::cout << "=== Astrix Client Simulation ===" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  i - Toggle GUI" << std::endl;
        std::cout << "  a - Toggle Aimbot" << std::endl;
        std::cout << "  k - Toggle KillAura" << std::endl;
        std::cout << "  e - Toggle ESP" << std::endl;
        std::cout << "  t - Toggle Triggerbot" << std::endl;
        std::cout << "  s - Toggle Speedwalk" << std::endl;
        std::cout << "  b - Toggle Boatfly" << std::endl;
        std::cout << "  q - Quit" << std::endl;
    }
    
    ~Simulator() {
        delete client;
    }
    
    void run() {
        while (running) {
            if (_kbhit()) {
                char key = _getch();
                switch (key) {
                    case 'i': client->onKeyPress(0x2D); break;
                    case 'a': client->onKeyPress('A'); break;
                    case 'k': client->onKeyPress('K'); break;
                    case 'e': client->onKeyPress('E'); break;
                    case 't': client->onKeyPress('T'); break;
                    case 's': client->onKeyPress('S'); break;
                    case 'b': client->onKeyPress('B'); break;
                    case 'q': running = false; break;
                }
            }
            
            // Simulate game tick
            Sleep(16);
        }
    }
};

int main() {
    Simulator simulator;
    simulator.run();
    return 0;
}

#endif

// ============================================================
// END OF COMPLETE SINGLE FILE
// ============================================================
