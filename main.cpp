#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <algorithm>
#include <optional>
#include <cstdint>

using namespace std;

int main()
{
    bool restartRequested = false;
    do
    {
        restartRequested = false;

        // LOAD BACKGROUND
        sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background.png"))
        return 1;

    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setPosition({0.f, 0.f});

    sf::Vector2u bgSize = backgroundTexture.getSize();
    float bgW = static_cast<float>(bgSize.x);
    float bgH = static_cast<float>(bgSize.y);

    // SPLASH SCREEN
    sf::RenderWindow splashWindow(sf::VideoMode({bgSize.x, bgSize.y}),
                                  "Super Pickleball Adventure - Press X to Start");
    splashWindow.setFramerateLimit(60);

    while (splashWindow.isOpen())
    {
        while (const auto event = splashWindow.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                splashWindow.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                if (key->code == sf::Keyboard::Key::X)
                    splashWindow.close();
        }
        splashWindow.clear();
        splashWindow.draw(backgroundSprite);
        splashWindow.display();
    }

    // MAIN GAME WINDOW
    sf::RenderWindow window(sf::VideoMode({bgSize.x, bgSize.y}),
                            "Super Pickleball Adventure");
    window.setFramerateLimit(60);

    // BG.png full court background
    sf::Texture bgCourtTexture;
    if (!bgCourtTexture.loadFromFile("BG.png"))
        return 1;

    sf::Sprite bgCourtSprite(bgCourtTexture);
    bgCourtSprite.setScale(sf::Vector2f(
        bgW / bgCourtTexture.getSize().x,
        bgH / bgCourtTexture.getSize().y
    ));
    bgCourtSprite.setPosition({0.f, 0.f});

    // Court geometry
    const float courtLeft       = 495.f;
    const float courtRight      = 880.f;
    const float courtCenter     = 690.f;
    const float netY            = 375.f;
    const float courtTopEdge    = 90.f;
    const float courtBottomEdge = 665.f;
    const float fieldLeft       = courtLeft  - 100.f;
    const float fieldRight      = courtRight + 100.f;
    const float deadZoneTop     = courtTopEdge    - 20.f;
    const float deadZoneBottom  = courtBottomEdge + 20.f;

    const float outsideLeft  = 493.f;
    const float outsideRight = 883.f;

    // Invisible collision shapes
    sf::RectangleShape court({courtRight - courtLeft, courtBottomEdge - courtTopEdge});
    court.setFillColor(sf::Color::Transparent);
    court.setOutlineColor(sf::Color::Transparent);
    court.setPosition({courtLeft, courtTopEdge});

    sf::RectangleShape netLine({courtRight - courtLeft, 10.f});
    netLine.setFillColor(sf::Color::Transparent);
    netLine.setPosition({courtLeft, netY - 5.f});

    sf::RectangleShape midLine({3.f, courtBottomEdge - courtTopEdge});
    midLine.setFillColor(sf::Color::Transparent);
    midLine.setPosition({courtCenter - 1.5f, courtTopEdge});

    // ── P1 SPRITE TEXTURES ────────────────────────────────────────────────────
    sf::Texture p1TexNormal, p1TexIdle,
                p1TexSwing1, p1TexSwing2,
                p1TexStep1,  p1TexStep2;

    if (!p1TexNormal.loadFromFile("P1.png"))        return 1;
    if (!p1TexIdle.loadFromFile("P1idle.png"))      return 1;
    if (!p1TexSwing1.loadFromFile("player1swing1.png")) return 1;
    if (!p1TexSwing2.loadFromFile("player1swing2.png")) return 1;
    if (!p1TexStep1.loadFromFile("p1step1.png"))    return 1;
    if (!p1TexStep2.loadFromFile("p1step2.png"))    return 1;

    // ── P2 SPRITE TEXTURES ────────────────────────────────────────────────────
    sf::Texture p2TexNormal, p2TexIdle,
                p2TexSwing1,
                p2TexStep1,  p2TexStep2,
                p2TexFwd1,   p2TexFwd2;

    if (!p2TexNormal.loadFromFile("P2.png"))           return 1;
    if (!p2TexIdle.loadFromFile("P2idle.png"))         return 1;
    if (!p2TexSwing1.loadFromFile("player2swing1.png")) return 1;
    if (!p2TexStep1.loadFromFile("p2step1.png"))       return 1;
    if (!p2TexStep2.loadFromFile("p2step2.png"))       return 1;
    if (!p2TexFwd1.loadFromFile("p2forward1.png"))     return 1;
    if (!p2TexFwd2.loadFromFile("p2forward2.png"))     return 1;

    // P1 visual sprite (replaces the red rectangle visually)
    sf::Sprite p1Sprite(p1TexNormal);
    p1Sprite.setOrigin(sf::Vector2f(
        p1TexNormal.getSize().x / 2.f,
        p1TexNormal.getSize().y / 2.f
    ));

    sf::Sprite p2Sprite(p2TexNormal);
    p2Sprite.setOrigin(sf::Vector2f(
        p2TexNormal.getSize().x / 2.f,
        p2TexNormal.getSize().y / 2.f
    ));

    // P1 forward/back textures
    sf::Texture p1TexFwd1, p1TexFwd2;
    if (!p1TexFwd1.loadFromFile("forward1.png")) return 1;
    if (!p1TexFwd2.loadFromFile("forward2.png")) return 1;

    // ── SCORE / SIGH TEXTURES (animations played when someone scores)
    sf::Texture p1ScoreTex1, p1ScoreTex2, p1SighTex;
    sf::Texture p2ScoreTex, p2ScoreTex2, p2SighTex;
    if (!p1ScoreTex1.loadFromFile("p1score1.png")) return 1;
    if (!p1ScoreTex2.loadFromFile("p1score2.png")) return 1;
    if (!p1SighTex.loadFromFile("p1sigh.png")) return 1;
    if (!p2ScoreTex.loadFromFile("p2score.png")) return 1;
    if (!p2ScoreTex2.loadFromFile("P2.png")) return 1;
    if (!p2SighTex.loadFromFile("p2sigh.png")) return 1;

    sf::Sprite p1ScoreSprite1(p1ScoreTex1);
    sf::Sprite p1ScoreSprite2(p1ScoreTex2);
    sf::Sprite p1SighSprite(p1SighTex);
    sf::Sprite p2ScoreSprite(p2ScoreTex);
    sf::Sprite p2ScoreSprite2(p2ScoreTex2);
    sf::Sprite p2SighSprite(p2SighTex);

    // P1 animation state
    enum class P1Anim { Idle, WalkSide, WalkFwd, Swing };
    P1Anim p1AnimState = P1Anim::Idle;

    // Idle breath
    float p1IdleTimer    = 0.f;
    float p1IdleInterval = 0.55f;
    int   p1IdleFrame    = 0;

    // Side walk cycle — 3-frame cycle: step1 -> normal -> step2
    float p1WalkTimer    = 0.f;
    float p1WalkInterval = 0.18f;
    int   p1WalkFrame    = 0;   // 0 = step1, 1 = normal, 2 = step2
    bool  p1FacingLeft   = false;

    // Forward/back walk cycle
    float p1FwdTimer    = 0.f;
    float p1FwdInterval = 0.18f;
    int   p1FwdFrame    = 0;    // 0 = forward1, 1 = forward2

    // Swing — ONE full cycle: swing1 then swing2, then done
    float p1SwingAnimTimer = 0.f;
    int   p1SwingAnimFrame = 0;
    bool  p1SwingAnimDone  = true;
    const float p1SwingFrameDur = 0.10f;

    // P2 animation state (mirrors P1 — same enum)
    P1Anim p2AnimState = P1Anim::Idle;

    // Idle
    float p2IdleTimer    = 0.f;
    float p2IdleInterval = 0.55f;
    int   p2IdleFrame    = 0;

    // Side walk — 3-frame cycle: step1 -> normal -> step2
    float p2WalkTimer    = 0.f;
    float p2WalkInterval = 0.18f;
    int   p2WalkFrame    = 0;
    bool  p2FacingLeft   = false;

    // Forward/back walk
    float p2FwdTimer    = 0.f;
    float p2FwdInterval = 0.18f;
    int   p2FwdFrame    = 0;

    // Swing — P2 only has 1 swing frame, plays once then done
    float p2SwingAnimTimer = 0.f;
    int   p2SwingAnimFrame = 0;
    bool  p2SwingAnimDone  = true;
    const float p2SwingFrameDur = 0.18f;  // slightly longer hold since only 1 frame

    // PLAYERS (rectangle kept as hitbox, invisible)
    sf::RectangleShape p1({30.f, 40.f});
    p1.setFillColor(sf::Color::Transparent);
    p1.setOrigin({15.f, 20.f});
    p1.setPosition({courtLeft + 295.f, courtBottomEdge});

    sf::RectangleShape p2({30.f, 40.f});
    p2.setFillColor(sf::Color::Transparent);
    p2.setOrigin({15.f, 20.f});
    p2.setPosition({courtLeft + 100.f, courtTopEdge});

    // BALL
sf::CircleShape ball(6.f);
    ball.setFillColor(sf::Color::Transparent);
  ball.setOrigin({6.f, 6.f});
    ball.setPosition({courtLeft + 295.f, courtBottomEdge - 35.f});

    sf::Texture ballTexture;
    if (!ballTexture.loadFromFile("ball.png"))
        return 1;

    sf::Sprite ballSprite(ballTexture);
    ballSprite.setOrigin(sf::Vector2f(ballTexture.getSize().x / 2.f,
                                      ballTexture.getSize().y / 2.f));

    sf::CircleShape ballShadow(9.f);
    ballShadow.setFillColor(sf::Color(0, 0, 0, 160));
    ballShadow.setOrigin({9.f, 4.5f});

    // Ball state
    float ballVx = 0.f;
    float ballVy = 0.f;
    float ballZ  = 0.f;
    float ballVz = 0.f;
    const float ballGravityZ = 600.f;
    const float bounceDampen = 0.52f;
    int   groundBounceCount  = 0;

    float ballSpin      = 0.f;
    float ballRotation  = 0.f;
    float vertSpinAngle = 0.f;
    float vertSpinSpeed = 0.f;

    float curveBouncePhase  = 0.f;
    bool  curveBounceActive = false;

    float curveForce   = 0.f;
    float curveTargetY = 0.f;
    bool  curveActive  = false;
    bool  curvePassed  = false;
    bool  curvePending = false;

    bool ballInPlay = false;
    bool ballOwner  = true;
    bool serving    = true;

    const float playerSpeed = 250.f;
    const float dashSpeed   = 700.f;
    const float dashTime    = 0.15f;

    bool  p1Dashing = false, p2Dashing = false;
    float p1DashTimer = 0.f, p2DashTimer = 0.f;
    float p1DashDirX  = 0.f, p1DashDirY  = 0.f;
    float p2DashDirX  = 0.f, p2DashDirY  = 0.f;

    bool  p1Swinging = false, p2Swinging = false;
    float p1SwingTimer = 0.f, p2SwingTimer = 0.f;
    const float swingDuration = 0.25f;

    float p1HitCooldown = 0.f, p2HitCooldown = 0.f;
    const float hitCooldown = 0.3f;

    int score1 = 0, score2 = 0;

    // Score animation state
    enum class ScoreState { None, P1Scored, P2Scored };
    ScoreState scoreState = ScoreState::None;
    float scoreStateTimer = 0.f;
    const float scoreStateDuration = 2.0f; // 2 seconds for animations

    const int maxScore = 10;
    bool gameOver = false;

    // P1 score animation (two frames)
    float p1ScoreFrameTimer = 0.f;
    const float p1ScoreFrameDur = 0.35f;
    int p1ScoreFrame = 0; // 0 or 1
    // P2 score animation (two frames)
    float p2ScoreFrameTimer = 0.f;
    const float p2ScoreFrameDur = 0.35f;
    int p2ScoreFrame = 0;

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return 1;

    sf::RectangleShape scorePanel({420.f, 72.f});
    scorePanel.setFillColor(sf::Color(0, 0, 0, 170));
    scorePanel.setOutlineThickness(2.f);
    scorePanel.setOutlineColor(sf::Color::White);
    scorePanel.setPosition({bgW / 2.f - 210.f, 14.f});

    sf::Text score1Text(font, "P1: 0", 32);
    score1Text.setFillColor(sf::Color::White);
    score1Text.setStyle(sf::Text::Bold);
    score1Text.setOutlineColor(sf::Color::Black);
    score1Text.setOutlineThickness(2.f);
    score1Text.setPosition({scorePanel.getPosition().x + 28.f, scorePanel.getPosition().y + 16.f});

    sf::Text score2Text(font, "P2: 0", 32);
    score2Text.setFillColor(sf::Color::White);
    score2Text.setStyle(sf::Text::Bold);
    score2Text.setOutlineColor(sf::Color::Black);
    score2Text.setOutlineThickness(2.f);
    score2Text.setPosition({scorePanel.getPosition().x + 262.f, scorePanel.getPosition().y + 16.f});

    sf::Text serveText(font, "X to Serve (P1)", 22);
    serveText.setFillColor(sf::Color::Yellow);
    serveText.setStyle(sf::Text::Bold);
    serveText.setPosition({courtLeft + 42.f, courtBottomEdge + 20.f});

    sf::Text restartText(font, "Press R to return to menu", 20);
    restartText.setFillColor(sf::Color::Yellow);
    restartText.setOutlineColor(sf::Color::Black);
    restartText.setOutlineThickness(1.f);
    restartText.setPosition({courtLeft + 80.f, courtBottomEdge + 50.f});

    sf::Text winText(font, "", 42);
    winText.setFillColor(sf::Color::Green);
    winText.setStyle(sf::Text::Bold);
    winText.setOutlineColor(sf::Color::Black);
    winText.setOutlineThickness(3.f);

    sf::Text winPrompt(font, "Press R to return to menu", 24);
    winPrompt.setFillColor(sf::Color::Yellow);
    winPrompt.setStyle(sf::Text::Bold);
    winPrompt.setOutlineColor(sf::Color::Black);
    winPrompt.setOutlineThickness(2.f);

    sf::CircleShape p2SwingCircle(35.f);
    p2SwingCircle.setFillColor(sf::Color(0, 200, 255, 120));
    p2SwingCircle.setOrigin({35.f, 35.f});

    // ── Helpers ───────────────────────────────────────────────────────────────
    auto aimIntoCourt = [&](float dirX, float fromX) -> float
    {
        if (fromX <= outsideLeft)  return  1.f;
        if (fromX >= outsideRight) return -1.f;
        return dirX;
    };

    auto isNearCourtSideEdge = [&](float x) -> bool
    {
        const float edgeThreshold = 50.f;
        return x <= courtLeft + edgeThreshold || x >= courtRight - edgeThreshold;
    };

    auto resetSpin = [&]()
    {
        ballSpin      = 0.f;
        ballRotation  = 0.f;
        vertSpinSpeed = 0.f;
        vertSpinAngle = 0.f;
        ballSprite.setRotation(sf::degrees(0.f));
    };

    auto hitToP2Field = [&](float dirX, bool dashing)
    {
        dirX = aimIntoCourt(dirX, ball.getPosition().x);
        float initialSide = 0.f;
        bool  edgeHit     = false;
        float bx = ball.getPosition().x;

        if (bx <= outsideLeft)
            initialSide = 120.f;
        else if (bx >= outsideRight)
            initialSide = -120.f;
        else if (isNearCourtSideEdge(bx) && dirX != 0.f)
        { initialSide = dirX * 45.f; edgeHit = true; }
        else
            initialSide = dirX * 45.f;

        ballVx = initialSide;
        ballVy = -420.f;
        ballZ  = 22.f;
        ballVz = 240.f;
        groundBounceCount = 0;
        curveBounceActive = false;
        curveBouncePhase  = 0.f;

        if (dashing)
        { vertSpinSpeed = 0.f; vertSpinAngle = 0.f; ballSpin = dirX * 900.f; }
        else
        {
            float p1Move = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) p1Move = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) p1Move =  1.f;
            if (dirX != 0.f)
            { vertSpinSpeed = 0.f; vertSpinAngle = 0.f; ballSpin = dirX * 300.f + p1Move * 160.f; }
            else
            { ballSpin = 0.f; vertSpinAngle = 0.f; vertSpinSpeed = -720.f; }
        }

        curveTargetY = courtTopEdge + 175.f;
        curveForce   = (dirX != 0.f) ? dirX * (dashing ? 220.f : 125.f) : 0.f;
        curvePending = edgeHit;
        curveActive  = !curvePending && (dirX != 0.f);
        curvePassed  = false;
        ballInPlay   = true;
    };

    auto hitToP1Field = [&](float dirX, bool dashing)
    {
        dirX = aimIntoCourt(dirX, ball.getPosition().x);
        float initialSide = 0.f;
        bool  edgeHit     = false;
        float bx = ball.getPosition().x;

        if (bx <= outsideLeft)
            initialSide = 120.f;
        else if (bx >= outsideRight)
            initialSide = -120.f;
        else if (isNearCourtSideEdge(bx) && dirX != 0.f)
        { initialSide = dirX * 45.f; edgeHit = true; }
        else
            initialSide = dirX * 45.f;

        ballVx = initialSide;
        ballVy = 420.f;
        ballZ  = 22.f;
        ballVz = 240.f;
        groundBounceCount = 0;
        curveBounceActive = false;
        curveBouncePhase  = 0.f;

        if (dashing)
        { vertSpinSpeed = 0.f; vertSpinAngle = 0.f; ballSpin = dirX * 900.f; }
        else
        {
            float p2Move = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  p2Move = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) p2Move =  1.f;
            if (dirX != 0.f)
            { vertSpinSpeed = 0.f; vertSpinAngle = 0.f; ballSpin = dirX * 300.f + p2Move * 160.f; }
            else
            { ballSpin = 0.f; vertSpinAngle = 0.f; vertSpinSpeed = 720.f; }
        }

        curveTargetY = courtTopEdge + 425.f;
        curveForce   = (dirX != 0.f) ? dirX * (dashing ? 220.f : 125.f) : 0.f;
        curvePending = edgeHit;
        curveActive  = !curvePending && (dirX != 0.f);
        curvePassed  = false;
        ballInPlay   = true;
    };

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        dt = min(dt, 0.033f);

        if (p1HitCooldown > 0.f) p1HitCooldown -= dt;
        if (p2HitCooldown > 0.f) p2HitCooldown -= dt;

        while (const optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->code == sf::Keyboard::Key::X && !ballInPlay && ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;
                    ballVx = 0.f; ballVy = -420.f; ballZ = 22.f; ballVz = 240.f;
                    groundBounceCount = 0; curveBounceActive = false; curveBouncePhase = 0.f;
                    ballSpin = (dirX != 0.f) ? dirX * 260.f : 0.f;
                    vertSpinSpeed = 0.f; vertSpinAngle = 0.f;
                    curveActive = false; curvePassed = false; curvePending = false;
                    p1Swinging       = true;
                    p1SwingTimer     = swingDuration;
                    p1SwingAnimFrame = 0;
                    p1SwingAnimTimer = 0.f;
                    p1SwingAnimDone  = false;
                    p1AnimState      = P1Anim::Swing;
                    ballInPlay = true; serving = false; serveText.setString("");
                }

                if (key->code == sf::Keyboard::Key::Period && !ballInPlay && !ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;
                    ballVx = 0.f; ballVy = 420.f; ballZ = 22.f; ballVz = 240.f;
                    groundBounceCount = 0; curveBounceActive = false; curveBouncePhase = 0.f;
                    ballSpin = (dirX != 0.f) ? dirX * 260.f : 0.f;
                    vertSpinSpeed = 0.f; vertSpinAngle = 0.f;
                    curveActive = false; curvePassed = false; curvePending = false;
                    p2Swinging       = true;
                    p2SwingTimer     = swingDuration;
                    p2SwingAnimFrame = 0;
                    p2SwingAnimTimer = 0.f;
                    p2SwingAnimDone  = false;
                    p2AnimState      = P1Anim::Swing;
                    ballInPlay = true; serving = false; serveText.setString("");
                }

                if (key->code == sf::Keyboard::Key::Q && !p1Dashing && ballInPlay)
                {
                    p1Dashing = true; p1DashTimer = dashTime;
                    p1DashDirX = 0.f; p1DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) p1DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) p1DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) p1DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) p1DashDirY =  1.f;
                    if (p1DashDirX == 0.f && p1DashDirY == 0.f) p1DashDirY = -1.f;
                }

                if (key->code == sf::Keyboard::Key::RShift && !p2Dashing && ballInPlay)
                {
                    p2Dashing = true; p2DashTimer = dashTime;
                    p2DashDirX = 0.f; p2DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  p2DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) p2DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    p2DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  p2DashDirY =  1.f;
                    if (p2DashDirX == 0.f && p2DashDirY == 0.f) p2DashDirY = 1.f;
                }

                if (key->code == sf::Keyboard::Key::Z && !p1Swinging)
                {
                    p1Swinging       = true;
                    p1SwingTimer     = swingDuration;
                    p1SwingAnimFrame = 0;
                    p1SwingAnimTimer = 0.f;
                    p1SwingAnimDone  = false;  // start fresh single cycle
                    p1AnimState      = P1Anim::Swing;
                }

                if (key->code == sf::Keyboard::Key::Slash && !p2Swinging)
                {
                    p2Swinging       = true;
                    p2SwingTimer     = swingDuration;
                    p2SwingAnimFrame = 0;
                    p2SwingAnimTimer = 0.f;
                    p2SwingAnimDone  = false;
                    p2AnimState      = P1Anim::Swing;
                }

                if (key->code == sf::Keyboard::Key::R)
                {
                    restartRequested = true;
                    window.close();
                }
            }
        }

        if (!gameOver)
        {
        // ── P1 MOVEMENT ───────────────────────────────────────────────────────
        bool p1Moving = false;
        float p1MoveX = 0.f;

        if (!p1Dashing)
        {
            float vx = 0.f, vy = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) { vx = -playerSpeed; p1MoveX = -1.f; p1Moving = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) { vx =  playerSpeed; p1MoveX =  1.f; p1Moving = true; }
            if (!ballOwner || ballInPlay)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) { vy = -playerSpeed; p1Moving = true; }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) { vy =  playerSpeed; p1Moving = true; }
            }
            p1.move({vx * dt, vy * dt});
        }
        else
        {
            p1DashTimer -= dt;
            p1.move({p1DashDirX * dashSpeed * dt, p1DashDirY * dashSpeed * dt});
            if (p1DashTimer <= 0.f) p1Dashing = false;
            p1Moving = true;
            p1MoveX  = p1DashDirX;
        }

        // P2 MOVEMENT
        if (!p2Dashing)
        {
            float vx = 0.f, vy = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  vx = -playerSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) vx =  playerSpeed;
            if (ballOwner || ballInPlay)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))   vy = -playerSpeed;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) vy =  playerSpeed;
            }
            p2.move({vx * dt, vy * dt});
        }
        else
        {
            p2DashTimer -= dt;
            p2.move({p2DashDirX * dashSpeed * dt, p2DashDirY * dashSpeed * dt});
            if (p2DashTimer <= 0.f) p2Dashing = false;
        }

        // CLAMP PLAYERS
        auto pos1 = p1.getPosition();
        if (!ballInPlay && ballOwner)
        {
            pos1.x = clamp(pos1.x, courtCenter, courtRight);
            pos1.y = courtBottomEdge;
        }
        else
        {
            pos1.x = clamp(pos1.x, fieldLeft, fieldRight);
            pos1.y = clamp(pos1.y, 496.f, courtBottomEdge);
        }
        p1.setPosition(pos1);

        auto pos2 = p2.getPosition();
        if (!ballInPlay && !ballOwner)
        {
            pos2.x = clamp(pos2.x, courtLeft, courtCenter);
            pos2.y = courtTopEdge;
        }
        else
        {
            pos2.x = clamp(pos2.x, fieldLeft, fieldRight);
            pos2.y = clamp(pos2.y, courtTopEdge, 224.f);
        }
        p2.setPosition(pos2);

        if (p1Swinging) { p1SwingTimer -= dt; if (p1SwingTimer <= 0.f) { p1Swinging = false; } }
        if (p2Swinging) { p2SwingTimer -= dt; if (p2SwingTimer <= 0.f) p2Swinging = false; }

      // ── P1 ANIMATION UPDATE ───────────────────────────────────────────────
        // Detect movement directions
        bool p1MovingLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        bool p1MovingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        bool p1MovingUp    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        bool p1MovingDown  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
        bool p1MovingSide  = p1MovingLeft || p1MovingRight;
        bool p1MovingFwd   = p1MovingUp   || p1MovingDown;

        if (p1Dashing)
        {
            p1MovingSide = (p1DashDirX != 0.f);
            p1MovingFwd  = (p1DashDirY != 0.f);
            if (p1DashDirX < 0.f) p1FacingLeft = true;
            if (p1DashDirX > 0.f) p1FacingLeft = false;
        }
        else
        {
            if (p1MovingLeft)  p1FacingLeft = true;
            if (p1MovingRight) p1FacingLeft = false;
        }

        // SWING — plays once: swing1 → swing2 → done
        if (p1AnimState == P1Anim::Swing)
        {
            if (!p1SwingAnimDone)
            {
                p1SwingAnimTimer += dt;
                if (p1SwingAnimTimer >= p1SwingFrameDur)
                {
                    p1SwingAnimTimer -= p1SwingFrameDur;
                    p1SwingAnimFrame++;
                    if (p1SwingAnimFrame >= 2)
                    {
                        p1SwingAnimDone = true;
                        p1SwingAnimFrame = 1; // hold last frame briefly
                        // Transition back
                        if (p1MovingSide)       p1AnimState = P1Anim::WalkSide;
                        else if (p1MovingFwd)   p1AnimState = P1Anim::WalkFwd;
                        else                    p1AnimState = P1Anim::Idle;
                    }
                }
            }
        }
        else if (p1MovingSide)
        {
            p1AnimState = P1Anim::WalkSide;
            p1FwdTimer  = 0.f;

            p1WalkTimer += dt;
            if (p1WalkTimer >= p1WalkInterval)
            {
                p1WalkTimer -= p1WalkInterval;
                p1WalkFrame = (p1WalkFrame + 1) % 3; // cycle 0 -> 1 -> 2
            }
        }
        else if (p1MovingFwd)
        {
            p1AnimState = P1Anim::WalkFwd;
            p1WalkTimer = 0.f;

            p1FwdTimer += dt;
            if (p1FwdTimer >= p1FwdInterval)
            {
                p1FwdTimer -= p1FwdInterval;
                p1FwdFrame  = 1 - p1FwdFrame; // alternate forward1 <-> forward2
            }
        }
        else
        {
            p1AnimState = P1Anim::Idle;
            p1WalkTimer = 0.f;
            p1FwdTimer  = 0.f;

            // reset walk frame when idle so walk starts predictably
            p1WalkFrame = 0;

            p1IdleTimer += dt;
            if (p1IdleTimer >= p1IdleInterval)
            {
                p1IdleTimer -= p1IdleInterval;
                p1IdleFrame  = 1 - p1IdleFrame;
            }
        }

        // Apply texture
        switch (p1AnimState)
        {
        case P1Anim::Swing:
            p1Sprite.setTexture(p1SwingAnimFrame == 0 ? p1TexSwing1 : p1TexSwing2);
            break;
        case P1Anim::WalkSide:
        {
            int displayFrame = p1FacingLeft ? (2 - p1WalkFrame) : p1WalkFrame;
            if (displayFrame == 0) p1Sprite.setTexture(p1TexStep1);
            else if (displayFrame == 1) p1Sprite.setTexture(p1TexNormal);
            else p1Sprite.setTexture(p1TexStep2);
        }
        break;
        case P1Anim::WalkFwd:
            p1Sprite.setTexture(p1FwdFrame == 0 ? p1TexFwd1 : p1TexFwd2);
            break;
        case P1Anim::Idle:
        default:
            p1Sprite.setTexture(p1IdleFrame == 0 ? p1TexNormal : p1TexIdle);
            break;
        }

        {
            auto texSize = p1Sprite.getTexture().getSize();
            float tw = static_cast<float>(texSize.x);
            float th = static_cast<float>(texSize.y);
            p1Sprite.setOrigin({tw / 2.f, th / 2.f});

           const float targetW = 72.f;
const float targetH = 96.f;
float baseScale = min(targetW / tw, targetH / th);
            bool mirror = p1FacingLeft;
            p1Sprite.setScale({ mirror ? -baseScale : baseScale, baseScale });
        }

        // Position sprite at same centre as the hitbox rectangle
        p1Sprite.setPosition(p1.getPosition());

        // ── P2 ANIMATION UPDATE ───────────────────────────────────────────────
        bool p2MovingLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool p2MovingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        bool p2MovingUp    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        bool p2MovingDown  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        bool p2MovingSide  = p2MovingLeft || p2MovingRight;
        bool p2MovingFwd   = p2MovingUp   || p2MovingDown;

        if (p2Dashing)
        {
            p2MovingSide = (p2DashDirX != 0.f);
            p2MovingFwd  = (p2DashDirY != 0.f);
            if (p2DashDirX < 0.f) p2FacingLeft = true;
            if (p2DashDirX > 0.f) p2FacingLeft = false;
        }
        else
        {
            if (p2MovingLeft)  p2FacingLeft = true;
            if (p2MovingRight) p2FacingLeft = false;
        }

        // SWING — P2 has only 1 frame; hold it for swingFrameDur then done
        if (p2AnimState == P1Anim::Swing)
        {
            if (!p2SwingAnimDone)
            {
                p2SwingAnimTimer += dt;
                if (p2SwingAnimTimer >= p2SwingFrameDur)
                {
                    p2SwingAnimDone  = true;
                    p2SwingAnimFrame = 0;
                    p2AnimState = p2MovingSide ? P1Anim::WalkSide
                                : p2MovingFwd  ? P1Anim::WalkFwd
                                :                P1Anim::Idle;
                }
            }
        }
        else if (p2MovingSide)
        {
            p2AnimState = P1Anim::WalkSide;
            p2FwdTimer  = 0.f;

            p2WalkTimer += dt;
            if (p2WalkTimer >= p2WalkInterval)
            {
                p2WalkTimer -= p2WalkInterval;
                p2WalkFrame  = (p2WalkFrame + 1) % 3;
            }
        }
        else if (p2MovingFwd)
        {
            p2AnimState = P1Anim::WalkFwd;
            p2WalkTimer = 0.f;

            p2FwdTimer += dt;
            if (p2FwdTimer >= p2FwdInterval)
            {
                p2FwdTimer -= p2FwdInterval;
                p2FwdFrame  = 1 - p2FwdFrame;
            }
        }
        else
        {
            p2AnimState = P1Anim::Idle;
            p2WalkTimer = 0.f;
            p2FwdTimer  = 0.f;
            p2WalkFrame = 0;

            p2IdleTimer += dt;
            if (p2IdleTimer >= p2IdleInterval)
            {
                p2IdleTimer -= p2IdleInterval;
                p2IdleFrame  = 1 - p2IdleFrame;
            }
        }

        // Apply P2 texture
        switch (p2AnimState)
        {
        case P1Anim::Swing:
            p2Sprite.setTexture(p2TexSwing1);
            break;
        case P1Anim::WalkSide:
        {
            int displayFrame = p2FacingLeft ? (2 - p2WalkFrame) : p2WalkFrame;
            if      (displayFrame == 0) p2Sprite.setTexture(p2TexStep1);
            else if (displayFrame == 1) p2Sprite.setTexture(p2TexNormal);
            else                        p2Sprite.setTexture(p2TexStep2);
            break;
        }
        case P1Anim::WalkFwd:
            p2Sprite.setTexture(p2FwdFrame == 0 ? p2TexFwd1 : p2TexFwd2);
            break;
        case P1Anim::Idle:
        default:
            p2Sprite.setTexture(p2IdleFrame == 0 ? p2TexNormal : p2TexIdle);
            break;
        }

        {
            auto texSize = p2Sprite.getTexture().getSize();
            float tw = static_cast<float>(texSize.x);
            float th = static_cast<float>(texSize.y);
            p2Sprite.setOrigin({tw / 2.f, th / 2.f});

            const float targetW = 72.f;
            const float targetH = 96.f;
            float baseScale = min(targetW / tw, targetH / th);
            bool mirror = p2FacingLeft;
            p2Sprite.setScale({ mirror ? -baseScale : baseScale, baseScale });
        }

        p2Sprite.setPosition(p2.getPosition());

        // ── SCORE/SIGH SPRITE SYNC — recompute every frame so position and scale
        //    always match the live p2Sprite, never a stale snapshot
        {
            const float targetW = 72.f;
            const float targetH = 96.f;
            bool mirror = p2FacingLeft;

            // P2 score sprites (shown when P2 scored)
            {
                auto texSize = p2ScoreTex.getSize();
                float tw = (float)texSize.x, th = (float)texSize.y;
                float baseScale = min(targetW / tw, targetH / th);
                const float p2OverlayScale = 1.30f;
                float sx = mirror ? -baseScale * p2OverlayScale : baseScale * p2OverlayScale;
                p2ScoreSprite.setOrigin({tw / 2.f, th / 2.f});
                p2ScoreSprite.setScale({sx, baseScale * p2OverlayScale});
                p2ScoreSprite.setPosition(p2.getPosition());
            }
            {
                auto texSize = p2ScoreTex2.getSize();
                float tw = (float)texSize.x, th = (float)texSize.y;
                float baseScale = min(targetW / tw, targetH / th);
                const float p2OverlayScale = 1.15f;
                float sx = mirror ? -baseScale * p2OverlayScale : baseScale * p2OverlayScale;
                p2ScoreSprite2.setOrigin({tw / 2.f, th / 2.f});
                p2ScoreSprite2.setScale({sx, baseScale * p2OverlayScale});
                p2ScoreSprite2.setPosition(p2.getPosition());
            }

            // P2 sigh sprite (shown when P1 scored)
            {
                auto texSize = p2SighTex.getSize();
                float tw = (float)texSize.x, th = (float)texSize.y;
                float baseScale = min(targetW / tw, targetH / th);
                const float p2OverlayScale = 1.30f;
                float sx = mirror ? -baseScale * p2OverlayScale : baseScale * p2OverlayScale;
                p2SighSprite.setOrigin({tw / 2.f, th / 2.f});
                p2SighSprite.setScale({sx, baseScale * p2OverlayScale});
                p2SighSprite.setPosition(p2.getPosition());
            }
        }

        // ── P1 SIGH SPRITE SYNC (shown when P2 scored) ───────────────────────────
        {
            const float targetW = 72.f;
            const float targetH = 96.f;
            bool mirror = p1FacingLeft;

            auto texSize = p1SighTex.getSize();
            float tw = (float)texSize.x, th = (float)texSize.y;
            float baseScale = min(targetW / tw, targetH / th);
            float sx = mirror ? -baseScale : baseScale;
            p1SighSprite.setOrigin({tw / 2.f, th / 2.f});
            p1SighSprite.setScale({sx, baseScale});
            p1SighSprite.setPosition(p1.getPosition());
        }

        // ── P1 SCORE SPRITES SYNC (shown when P1 scored) ─────────────────────────
        {
            const float targetW = 72.f;
            const float targetH = 96.f;
            bool mirror = p1FacingLeft;

            auto syncScoreSprite = [&](sf::Sprite& spr, const sf::Texture& tex)
            {
                auto texSize = tex.getSize();
                float tw = (float)texSize.x, th = (float)texSize.y;
                float baseScale = min(targetW / tw, targetH / th);
                float sx = mirror ? -baseScale : baseScale;
                spr.setOrigin({tw / 2.f, th / 2.f});
                spr.setScale({sx, baseScale});
                spr.setPosition(p1.getPosition());
            };
            syncScoreSprite(p1ScoreSprite1, p1ScoreTex1);
            syncScoreSprite(p1ScoreSprite2, p1ScoreTex2);
        }

        // BALL FOLLOW SERVER
        if (!ballInPlay)
        {
            ballZ  = 0.f;
            ballVz = 0.f;
            groundBounceCount = 0;
            resetSpin();
            if (ballOwner)
                ball.setPosition({p1.getPosition().x, p1.getPosition().y - 30.f});
            else
                ball.setPosition({p2.getPosition().x, p2.getPosition().y + 30.f});
        }

        // ── BALL PHYSICS ──────────────────────────────────────────────────────
        if (ballInPlay)
        {
            auto bpos = ball.getPosition();

            if (curvePending)
            {
                bool reached = (ballVy < 0.f && bpos.y <= curveTargetY) ||
                               (ballVy > 0.f && bpos.y >= curveTargetY);
                if (reached) { curvePending = false; curveActive = (curveForce != 0.f); }
            }

            if (curveActive)
            {
                bool reached = (ballVy < 0.f && bpos.y <= curveTargetY) ||
                               (ballVy > 0.f && bpos.y >= curveTargetY);

                if (!curvePassed && reached)
                {
                    ballVx += curveForce * dt * 30.f;
                    ballVx  = clamp(ballVx, -320.f, 320.f);

                    if (abs(ballVx) >= abs(curveForce) * 0.75f)
                    {
                        ballVx            = curveForce;
                        curvePassed       = true;
                        curveActive       = false;
                        curveBounceActive = true;
                        curveBouncePhase  = -3.14159265f / 2.f;

                        if (abs(curveForce) >= 180.f)
                        {
                            ballVy          *= 1.4f;
                            ballVx          *= 1.1f;
                            ballVx           = clamp(ballVx, -500.f, 500.f);
                            ballSpin        *= 2.2f;
                            curveBouncePhase = -3.14159265f;
                        }
                    }
                }
            }

            if (bpos.x <= outsideLeft  && ballVx < 0.f) ballVx += 80.f * dt;
            if (bpos.x >= outsideRight && ballVx > 0.f) ballVx -= 80.f * dt;

            if (bpos.x <= fieldLeft)
            { ball.setPosition({fieldLeft + 1.f, bpos.y}); ballVx = abs(ballVx) * 0.5f; }
            if (bpos.x >= fieldRight)
            { ball.setPosition({fieldRight - 1.f, bpos.y}); ballVx = -abs(ballVx) * 0.5f; }

            ballVz -= ballGravityZ * dt;
            ballZ  += ballVz * dt;

            if (ballZ <= 0.f && ballVz < 0.f)
            {
                ballZ = 0.f;
                groundBounceCount++;
                float dampThisBounce = bounceDampen * (1.f - groundBounceCount * 0.12f);
                dampThisBounce       = max(dampThisBounce, 0.f);
                ballVz               = -ballVz * dampThisBounce;
                if (ballVz < 30.f) { ballVz = 0.f; ballZ = 0.f; }
            }

            if (vertSpinSpeed != 0.f)
            {
                vertSpinAngle += vertSpinSpeed * dt;
                vertSpinSpeed -= vertSpinSpeed * dt * 1.8f;
                if (abs(vertSpinSpeed) < 2.f) vertSpinSpeed = 0.f;
            }
            else if (vertSpinAngle != 0.f)
            {
                vertSpinAngle -= vertSpinAngle * dt * 4.f;
                if (abs(vertSpinAngle) < 0.5f) vertSpinAngle = 0.f;
            }

            if (curveBounceActive)
            {
                curveBouncePhase += dt * 8.f;
                if (curveBouncePhase >= 3.14159265f)
                { curveBouncePhase = 3.14159265f; curveBounceActive = false; }
            }

            ballVx = clamp(ballVx, -550.f, 550.f);
            ballVy = clamp(ballVy, -600.f, 600.f);

            float nextX = clamp(bpos.x + ballVx * dt, fieldLeft, fieldRight);
            float nextY = bpos.y + ballVy * dt;
            ball.setPosition(sf::Vector2f(nextX, nextY));
            bpos = ball.getPosition();

            if (bpos.y < deadZoneTop)
            {
                score1++;
                score1Text.setString("P1: " + to_string(score1));
                ballInPlay = false; ballOwner = true;
                serving = true; curveActive = false; curvePending = false;
                if (score1 >= maxScore)
                {
                    gameOver = true;
                    serveText.setString("");
                }
                else
                {
                    serveText.setString("X to Serve (P1)");
                    serveText.setPosition({courtLeft + 80.f, courtBottomEdge + 20.f});
                }
                // Trigger score animations: P1 scored -> show P1 score animation and P2 sigh
                scoreState = ScoreState::P1Scored;
                scoreStateTimer = scoreStateDuration;
                p1ScoreFrame = 0; p1ScoreFrameTimer = 0.f;
            }
            if (bpos.y > deadZoneBottom)
            {
                score2++;
                score2Text.setString("P2: " + to_string(score2));
                ballInPlay = false; ballOwner = false;
                serving = true; curveActive = false; curvePending = false;
                if (score2 >= maxScore)
                {
                    gameOver = true;
                    serveText.setString("");
                }
                else
                {
                    serveText.setString(". to Serve (P2)");
                    serveText.setPosition({courtLeft + 80.f, courtTopEdge - 50.f});
                }
                // Trigger score animations: P2 scored -> show P2 score animation and P1 sigh
                scoreState = ScoreState::P2Scored;
                scoreStateTimer = scoreStateDuration;
            }

            sf::FloatRect ballB = ball.getGlobalBounds();

            sf::FloatRect p1Hit = p1.getGlobalBounds();
            p1Hit.position.x -= 20.f; p1Hit.size.x += 40.f;
            p1Hit.position.y -= 20.f; p1Hit.size.y += 40.f;

            sf::FloatRect p2Hit = p2.getGlobalBounds();
            p2Hit.position.x -= 20.f; p2Hit.size.x += 40.f;
            p2Hit.position.y -= 20.f; p2Hit.size.y += 40.f;

            bool p1CanHit = (p1Swinging || p1Dashing) &&
                             p1HitCooldown <= 0.f &&
                             ball.getPosition().y > netY + 25.f;
            if (p1CanHit && ballB.findIntersection(p1Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;
                hitToP2Field(dirX, p1Dashing);
              p1Swinging       = false;
                p1HitCooldown    = hitCooldown;
                p1SwingAnimFrame = 0;
                p1SwingAnimTimer = 0.f;
                p1SwingAnimDone  = false;
                p1AnimState      = P1Anim::Swing;
            }

            bool p2CanHit = (p2Swinging || p2Dashing) &&
                             p2HitCooldown <= 0.f &&
                             ball.getPosition().y < netY - 25.f;
            if (p2CanHit && ballB.findIntersection(p2Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;
                hitToP1Field(dirX, p2Dashing);
                p2Swinging = false; p2HitCooldown = hitCooldown;
                p2SwingAnimFrame = 0;
                p2SwingAnimTimer = 0.f;
                p2SwingAnimDone  = false;
                p2AnimState      = P1Anim::Swing;
            }
        }
        }

        // Update score animation timers
        if (scoreState != ScoreState::None)
        {
            scoreStateTimer -= dt;
            if (scoreState == ScoreState::P1Scored)
            {
                p1ScoreFrameTimer += dt;
                if (p1ScoreFrameTimer >= p1ScoreFrameDur)
                {
                    p1ScoreFrameTimer -= p1ScoreFrameDur;
                    p1ScoreFrame = 1 - p1ScoreFrame;
                }
            }
            if (scoreState == ScoreState::P2Scored)
            {
                p2ScoreFrameTimer += dt;
                if (p2ScoreFrameTimer >= p2ScoreFrameDur)
                {
                    p2ScoreFrameTimer -= p2ScoreFrameDur;
                    p2ScoreFrame = 1 - p2ScoreFrame;
                }
            }
            if (scoreStateTimer <= 0.f)
                scoreState = ScoreState::None;
        }

        // ── DRAW ──────────────────────────────────────────────────────────────
        window.clear();
        window.draw(bgCourtSprite);

        float heightRatio = clamp(ballZ / 120.f, 0.f, 1.f);

        if (ballInPlay)
        {
            float shadowBaseScale = clamp(1.f - heightRatio * 0.30f, 0.70f, 1.f);
            int shadowAlpha = (int)(220.f - heightRatio * 80.f);
            if (shadowAlpha < 80)  shadowAlpha = 80;
            if (shadowAlpha > 220) shadowAlpha = 220;
            ballShadow.setFillColor(sf::Color(0, 0, 0, shadowAlpha));
            ballShadow.setScale(sf::Vector2f(shadowBaseScale, shadowBaseScale * 0.42f));
            ballShadow.setPosition(sf::Vector2f(ball.getPosition().x,
                                                ball.getPosition().y + 8.f));
            window.draw(ballShadow);
        }

        // Draw P1 and P2 sprites instead of rectangles
        // During a score animation we replace the base sprites with the
        // appropriate score/sigh sprites so characters don't duplicate.
        if (scoreState == ScoreState::P1Scored)
        {
            // P1: show score animation instead of base sprite
            // P2: show sigh instead of base sprite
            // do not draw p1Sprite or p2Sprite here
        }
        else if (scoreState == ScoreState::P2Scored)
        {
            // P2: show score animation instead of base sprite
            // P1: show sigh instead of base sprite
            // do not draw p1Sprite or p2Sprite here
        }
        else
        {
            window.draw(p1Sprite);
            window.draw(p2Sprite);
        }

       float spriteScale = 0.52f + heightRatio * 0.12f;

        if (curveBounceActive)
        {
            float bounceAmount = sin(curveBouncePhase);
            spriteScale *= 1.f + 0.08f * bounceAmount;
        }

        float squishX = 1.f, squishY = 1.f;
        if (ballZ < 5.f && ballVz < -60.f && ballInPlay)
        {
            float t = 1.f - (ballZ / 5.f);
            squishX = 1.f + t * 0.12f;
            squishY = 1.f - t * 0.10f;
        }

        {
            float spinRate = 0.f;
            if (vertSpinSpeed != 0.f || abs(vertSpinAngle) > 0.5f)
            {
                float velDriven  = ballVy * 0.55f;
                float kickDriven = vertSpinSpeed * 0.18f;
                spinRate = (abs(vertSpinSpeed) > 2.f) ? kickDriven : velDriven;
            }
            else
            {
                float dominantSpin = (abs(ballVx) > abs(ballVy))
                                     ? ballVx * 0.55f
                                     : ballVy * 0.35f;
                spinRate = dominantSpin;
            }

            ballRotation += spinRate * dt;
            if (ballRotation >  360.f) ballRotation -= 360.f;
            if (ballRotation < -360.f) ballRotation += 360.f;

            ballSprite.setRotation(sf::degrees(ballRotation));
            ballSprite.setScale(sf::Vector2f(spriteScale * squishX, spriteScale * squishY));
        }

        ballSprite.setPosition(sf::Vector2f(
            ball.getPosition().x,
            ball.getPosition().y - ballZ * 0.62f
        ));
        if (ballInPlay)
            window.draw(ballSprite);

        // Draw score/sigh animations if active
        if (scoreState == ScoreState::P1Scored)
        {
            // draw P1 score (two-frame animation) and P2 sigh
            if (p1ScoreFrame == 0) window.draw(p1ScoreSprite1);
            else                   window.draw(p1ScoreSprite2);
            window.draw(p2SighSprite);
        }
        else if (scoreState == ScoreState::P2Scored)
        {
            if (p2ScoreFrame == 0) window.draw(p2ScoreSprite);
            else                    window.draw(p2ScoreSprite2);
            window.draw(p1SighSprite);
        }

        window.draw(scorePanel);
        window.draw(score1Text);
        window.draw(score2Text);
        window.draw(serveText);
        window.draw(restartText);

        if (gameOver)
        {
            std::string winnerStr = (score1 >= maxScore) ? "P1 Wins!" : "P2 Wins!";
            winText.setString(winnerStr);
            winText.setPosition({bgW * 0.5f - 140.f, bgH * 0.45f});
            window.draw(winText);

            winPrompt.setString("Press R to return to menu");
            winPrompt.setPosition({bgW * 0.5f - 180.f, bgH * 0.55f});
            window.draw(winPrompt);
        }

        window.display();
    }

    } while (restartRequested);

    return 0;
}
