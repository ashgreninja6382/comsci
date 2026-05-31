#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <optional>
#include <cstdint>

using namespace std;

int main()
{
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

    // P1 visual sprite (replaces the red rectangle visually)
    sf::Sprite p1Sprite(p1TexNormal);
    p1Sprite.setOrigin(sf::Vector2f(
        p1TexNormal.getSize().x / 2.f,
        p1TexNormal.getSize().y / 2.f   // bottom-centre origin to match rect
    ));

  // P1 forward/back textures
    sf::Texture p1TexFwd1, p1TexFwd2;
    if (!p1TexFwd1.loadFromFile("forward1.png")) return 1;
    if (!p1TexFwd2.loadFromFile("forward2.png")) return 1;

    // P1 animation state
    enum class P1Anim { Idle, WalkSide, WalkFwd, Swing };
    P1Anim p1AnimState = P1Anim::Idle;

    // Idle breath
    float p1IdleTimer    = 0.f;
    float p1IdleInterval = 0.55f;
    int   p1IdleFrame    = 0;

    // Side walk cycle — alternates step1/step2 together (both legs alternate)
    float p1WalkTimer    = 0.f;
    float p1WalkInterval = 0.18f;
    int   p1WalkFrame    = 0;   // 0 = step1, 1 = step2
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

    // PLAYERS (rectangle kept as hitbox, invisible)
    sf::RectangleShape p1({30.f, 40.f});
    p1.setFillColor(sf::Color::Transparent);
    p1.setOrigin({15.f, 20.f});
    p1.setPosition({courtLeft + 295.f, courtBottomEdge});

    sf::RectangleShape p2({30.f, 40.f});
    p2.setFillColor(sf::Color(128, 0, 200));
    p2.setOrigin({15.f, 20.f});
    p2.setPosition({courtLeft + 100.f, courtTopEdge});

    // BALL
    sf::CircleShape ball(10.f);
    ball.setFillColor(sf::Color::Transparent);
    ball.setOrigin({10.f, 10.f});
    ball.setPosition({courtLeft + 295.f, courtBottomEdge - 35.f});

    sf::Texture ballTexture;
    if (!ballTexture.loadFromFile("ball.png"))
        return 1;

    sf::Sprite ballSprite(ballTexture);
    ballSprite.setOrigin(sf::Vector2f(ballTexture.getSize().x / 2.f,
                                      ballTexture.getSize().y / 2.f));

    sf::CircleShape ballShadow(14.f);
    ballShadow.setFillColor(sf::Color(0, 0, 0, 160));
    ballShadow.setOrigin({14.f, 7.f});

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

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return 1;

    sf::Text score1Text(font, "P1: 0", 28);
    score1Text.setFillColor(sf::Color::White);
    score1Text.setPosition({courtLeft - 80.f, courtBottomEdge + 20.f});

    sf::Text score2Text(font, "P2: 0", 28);
    score2Text.setFillColor(sf::Color::White);
    score2Text.setPosition({courtRight - 180.f, courtTopEdge - 50.f});

    sf::Text serveText(font, "X to Serve (P1)", 20);
    serveText.setFillColor(sf::Color::Yellow);
    serveText.setPosition({courtLeft + 80.f, courtBottomEdge + 20.f});

    sf::CircleShape p1SwingCircle(35.f);
    p1SwingCircle.setFillColor(sf::Color(255, 255, 0, 120));
    p1SwingCircle.setOrigin({35.f, 35.f});

    sf::CircleShape p2SwingCircle(35.f);
    p2SwingCircle.setFillColor(sf::Color(0, 200, 255, 120));
    p2SwingCircle.setOrigin({35.f, 35.f});

    sf::Text p1PosText(font, "P1 (0, 0)", 16);
    p1PosText.setFillColor(sf::Color::Yellow);
    p1PosText.setPosition({courtLeft - 80.f, courtBottomEdge - 40.f});

    sf::Text p2PosText(font, "P2 (0, 0)", 16);
    p2PosText.setFillColor(sf::Color::Cyan);
    p2PosText.setPosition({courtLeft - 80.f, courtTopEdge + 40.f});

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
                { p2Swinging = true; p2SwingTimer = swingDuration; }
            }
        }

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
            pos1.y = clamp(pos1.y, courtTopEdge + 360.f, courtBottomEdge);
        }
        else
        {
            pos1.x = clamp(pos1.x, fieldLeft, fieldRight);
            pos1.y = clamp(pos1.y, netY + 20.f, deadZoneBottom - 20.f);
        }
        p1.setPosition(pos1);

        auto pos2 = p2.getPosition();
        if (!ballInPlay && !ballOwner)
        {
            pos2.x = clamp(pos2.x, courtLeft, courtCenter);
            pos2.y = clamp(pos2.y, courtTopEdge, courtTopEdge + 230.f);
        }
        else
        {
            pos2.x = clamp(pos2.x, fieldLeft, fieldRight);
            pos2.y = clamp(pos2.y, deadZoneTop + 20.f, netY - 20.f);
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
                p1WalkFrame  = 1 - p1WalkFrame; // alternate step1 <-> step2
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
            p1Sprite.setTexture(p1WalkFrame == 0 ? p1TexStep1 : p1TexStep2);
            break;
        case P1Anim::WalkFwd:
            p1Sprite.setTexture(p1FwdFrame == 0 ? p1TexFwd1 : p1TexFwd2);
            break;
        case P1Anim::Idle:
        default:
            p1Sprite.setTexture(p1IdleFrame == 0 ? p1TexNormal : p1TexIdle);
            break;
        }

        // Flip horizontally when facing left (side walk only)
        {
            auto texSize = p1Sprite.getTexture().getSize();
            float tw = static_cast<float>(texSize.x);
            float th = static_cast<float>(texSize.y);
            p1Sprite.setOrigin({tw / 2.f, th / 2.f});

            // Only flip for side movement — forward/idle/swing face forward
            if (p1AnimState == P1Anim::WalkSide && p1FacingLeft)
                p1Sprite.setScale({-1.f, 1.f});
            else
                p1Sprite.setScale({ 1.f, 1.f});
        }

        // Position sprite at same centre as the hitbox rectangle
        p1Sprite.setPosition(p1.getPosition());

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
                ballInPlay = false; ballOwner = false;
                serving = true; curveActive = false; curvePending = false;
                p2.setPosition({courtLeft + 100.f, courtTopEdge});
                serveText.setString(". to Serve (P2)");
                serveText.setPosition({courtLeft + 80.f, courtTopEdge - 50.f});
            }
            if (bpos.y > deadZoneBottom)
            {
                score2++;
                score2Text.setString("P2: " + to_string(score2));
                ballInPlay = false; ballOwner = true;
                serving = true; curveActive = false; curvePending = false;
                p1.setPosition({courtLeft + 295.f, courtBottomEdge});
                serveText.setString("X to Serve (P1)");
                serveText.setPosition({courtLeft + 80.f, courtBottomEdge + 20.f});
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
            }
        }

        // ── DRAW ──────────────────────────────────────────────────────────────
        window.clear();
        window.draw(bgCourtSprite);

        // P1 swing circle removed — using sprite animation instead
        if (p2Swinging) { p2SwingCircle.setPosition(p2.getPosition()); window.draw(p2SwingCircle); }

        float heightRatio = clamp(ballZ / 120.f, 0.f, 1.f);

        float shadowBaseScale = clamp(1.f - heightRatio * 0.30f, 0.70f, 1.f);
        int shadowAlpha = (int)(220.f - heightRatio * 80.f);
        if (shadowAlpha < 80)  shadowAlpha = 80;
        if (shadowAlpha > 220) shadowAlpha = 220;
        ballShadow.setFillColor(sf::Color(0, 0, 0, shadowAlpha));
        ballShadow.setScale(sf::Vector2f(shadowBaseScale, shadowBaseScale * 0.42f));
        ballShadow.setPosition(sf::Vector2f(ball.getPosition().x,
                                            ball.getPosition().y + 8.f));
        window.draw(ballShadow);

        // Draw P1 sprite instead of rectangle
        window.draw(p1Sprite);
        window.draw(p2);   // P2 still uses rectangle

        float spriteScale = 0.82f + heightRatio * 0.18f;

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
        window.draw(ballSprite);

        window.draw(score1Text);
        window.draw(score2Text);
        window.draw(serveText);

        p1PosText.setString("P1 (" + to_string((int)p1.getPosition().x) + ", " +
                            to_string((int)p1.getPosition().y) + ")");
        p2PosText.setString("P2 (" + to_string((int)p2.getPosition().x) + ", " +
                            to_string((int)p2.getPosition().y) + ")");
        window.draw(p1PosText);
        window.draw(p2PosText);

        window.display();
    }

    return 0;
}
