#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <optional>

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

    // FIELD
    sf::RectangleShape field({bgW, bgH});
    field.setFillColor(sf::Color(34, 100, 34));

    // Court centered in window — always 400x600
    float courtOffsetX = (bgW - 400.f) / 2.f;
    float courtOffsetY = (bgH - 600.f) / 2.f;

    // COURT
    sf::RectangleShape court({400.f, 600.f});
    court.setFillColor(sf::Color(150, 180, 150));
    court.setOutlineColor(sf::Color::White);
    court.setOutlineThickness(3.f);
    court.setPosition({courtOffsetX, courtOffsetY});

    sf::RectangleShape netLine({400.f, 10.f});
    netLine.setFillColor(sf::Color::White);
    netLine.setPosition({courtOffsetX, courtOffsetY + 295.f});

    sf::RectangleShape midLine({3.f, 600.f});
    midLine.setFillColor(sf::Color::White);
    midLine.setPosition({courtOffsetX + 198.5f, courtOffsetY});

    // PLAYERS
    sf::RectangleShape p1({30.f, 40.f});
    p1.setFillColor(sf::Color::Red);
    p1.setOrigin({15.f, 20.f});
    p1.setPosition({courtOffsetX + 295.f, courtOffsetY + 580.f});

    sf::RectangleShape p2({30.f, 40.f});
    p2.setFillColor(sf::Color(128, 0, 200));
    p2.setOrigin({15.f, 20.f});
    p2.setPosition({courtOffsetX + 100.f, courtOffsetY + 20.f});

    // BALL
    sf::CircleShape ball(10.f);
    ball.setFillColor(sf::Color::White);
    ball.setOutlineColor(sf::Color::Black);
    ball.setOutlineThickness(2.f);
    ball.setOrigin({10.f, 10.f});
    ball.setPosition({courtOffsetX + 295.f, courtOffsetY + 550.f});

    sf::Texture ballTexture;
    if (!ballTexture.loadFromFile("ball.png"))
        return 1;

    sf::Sprite ballSprite(ballTexture);
    ballSprite.setOrigin(sf::Vector2f(ballTexture.getSize().x / 2.f,
                                      ballTexture.getSize().y / 2.f));
    ballSprite.setPosition(ball.getPosition());

    sf::CircleShape ballShadow(12.f);
    ballShadow.setFillColor(sf::Color(0, 0, 0, 120));
    ballShadow.setOrigin({12.f, 6.f});

    float ballVx = 0.f;
    float ballVy = 0.f;
    float ballGravity = 320.f;
    float ballZ = 0.f;
    float ballVz = 0.f;
    float ballSpin = 0.f;
    const float ballGravityZ = 600.f;
    float curveBouncePhase = 0.f;
    bool  curveBounceActive = false;

    // CURVE SYSTEM
    float curveForce   = 0.f;
    float curveTargetY = 0.f;
    bool  curveActive  = false;
    bool  curvePassed  = false;

    bool ballInPlay = false;
    bool ballOwner  = true;
    bool serving    = true;

    // SPEEDS
    const float playerSpeed = 250.f;
    const float dashSpeed   = 700.f;
    const float dashTime    = 0.15f;

    // DASH
    bool  p1Dashing = false, p2Dashing = false;
    float p1DashTimer = 0.f, p2DashTimer = 0.f;
    float p1DashDirX = 0.f,  p1DashDirY = 0.f;
    float p2DashDirX = 0.f,  p2DashDirY = 0.f;

    // SWING
    bool  p1Swinging = false, p2Swinging = false;
    float p1SwingTimer = 0.f, p2SwingTimer = 0.f;
    const float swingDuration = 0.25f;

    // HIT COOLDOWN
    float p1HitCooldown = 0.f, p2HitCooldown = 0.f;
    const float hitCooldown = 0.3f;

    // BOUNDARIES — all relative to centered court
    const float deadZoneTop    = courtOffsetY - 20.f;
    const float deadZoneBottom = courtOffsetY + 620.f;
    const float fieldLeft      = courtOffsetX - 80.f;
    const float fieldRight     = courtOffsetX + 480.f;
    const float courtLeft      = courtOffsetX;
    const float courtRight     = courtOffsetX + 400.f;
    const float courtCenter    = courtOffsetX + 200.f;
    const float courtLeftEdge  = courtOffsetX + 15.f;
    const float courtRightEdge = courtOffsetX + 385.f;

    int score1 = 0, score2 = 0;

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
        return 1;

    sf::Text score1Text(font, "P1: 0", 28);
    score1Text.setFillColor(sf::Color::White);
    score1Text.setPosition({courtOffsetX - 80.f, courtOffsetY + 620.f});

    sf::Text score2Text(font, "P2: 0", 28);
    score2Text.setFillColor(sf::Color::White);
    score2Text.setPosition({courtOffsetX + 320.f, courtOffsetY - 80.f});

    sf::Text serveText(font, "X to Serve (P1)", 20);
    serveText.setFillColor(sf::Color::Yellow);
    serveText.setPosition({courtOffsetX + 80.f, courtOffsetY + 630.f});

    sf::CircleShape p1SwingCircle(35.f);
    p1SwingCircle.setFillColor(sf::Color(255, 255, 0, 120));
    p1SwingCircle.setOrigin({35.f, 35.f});

    sf::CircleShape p2SwingCircle(35.f);
    p2SwingCircle.setFillColor(sf::Color(0, 200, 255, 120));
    p2SwingCircle.setOrigin({35.f, 35.f});

    sf::Text p1PosText(font, "P1 (0, 0)", 16);
    p1PosText.setFillColor(sf::Color::Yellow);
    p1PosText.setPosition({courtOffsetX - 80.f, courtOffsetY + 590.f});

    sf::Text p2PosText(font, "P2 (0, 0)", 16);
    p2PosText.setFillColor(sf::Color::Cyan);
    p2PosText.setPosition({courtOffsetX - 80.f, courtOffsetY - 60.f});

    auto aimIntoCourt = [&](float dirX, float fromX) -> float
    {
        if (fromX < courtLeft)  return 1.f;
        if (fromX > courtRight) return -1.f;
        return dirX;
    };

    // HIT TO P2 FIELD
    auto hitToP2Field = [&](float dirX, bool dashing)
    {
        dirX = aimIntoCourt(dirX, ball.getPosition().x);
        float power = 420.f;
        float initialSide = 0.f;
        if (ball.getPosition().x < courtLeft)
            initialSide = 120.f;
        else if (ball.getPosition().x > courtRight)
            initialSide = -120.f;
        else
           initialSide = dirX * 45.f;

        ballVx = initialSide;
        ballVy = -power;
        ballZ  = 22.f;
        ballVz = 240.f;
        curveBounceActive = false;
        curveBouncePhase  = 0.f;

        if (dashing)
            ballSpin = dirX * 900.f;
        else
        {
            float p1Move = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) p1Move = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) p1Move =  1.f;
            if (dirX != 0.f)
                ballSpin = dirX * 300.f + p1Move * 160.f;
            else
                ballSpin = -700.f;
        }

        curveTargetY = courtOffsetY + 175.f;
        curveForce   = (dirX != 0.f) ? dirX * (dashing ? 220.f : 125.f) : 0.f;
        curveActive  = (dirX != 0.f);
        curvePassed  = false;
        ballInPlay   = true;
    };

    // HIT TO P1 FIELD
    auto hitToP1Field = [&](float dirX, bool dashing)
    {
        dirX = aimIntoCourt(dirX, ball.getPosition().x);
        float power = 420.f;
        float initialSide = 0.f;
        if (ball.getPosition().x < courtLeft)
            initialSide = 120.f;
        else if (ball.getPosition().x > courtRight)
            initialSide = -120.f;
        else
           initialSide = dirX * 45.f;

        ballVx = initialSide;
        ballVy = power;
        ballZ  = 22.f;
        ballVz = 240.f;
        curveBounceActive = false;
        curveBouncePhase  = 0.f;

        if (dashing)
            ballSpin = dirX * 900.f;
        else
        {
            float p2Move = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  p2Move = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) p2Move =  1.f;
            if (dirX != 0.f)
                ballSpin = dirX * 300.f + p2Move * 160.f;
            else
                ballSpin = 700.f;
        }

        curveTargetY = courtOffsetY + 425.f;
        curveForce   = (dirX != 0.f) ? dirX * (dashing ? 220.f : 125.f) : 0.f;
        curveActive  = (dirX != 0.f);
        curvePassed  = false;
        ballInPlay   = true;
    };

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        if (p1HitCooldown > 0.f) p1HitCooldown -= dt;
        if (p2HitCooldown > 0.f) p2HitCooldown -= dt;

        while (const optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                // P1 SERVE
                if (key->code == sf::Keyboard::Key::X &&
                    !ballInPlay && ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;

                    ballVx = 0.f;
                    ballVy = -420.f;
                    ballZ  = 22.f;
                    ballVz = 240.f;
                    curveBounceActive = false;
                    curveBouncePhase  = 0.f;
                    ballSpin    = (dirX != 0.f) ? dirX * 260.f : 0.f;
                    curveActive = false;
                    curvePassed = false;
                    ballInPlay  = true;
                    serving     = false;
                    serveText.setString("");
                }

                // P2 SERVE
                if (key->code == sf::Keyboard::Key::Period &&
                    !ballInPlay && !ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;

                    ballVx = 0.f;
                    ballVy = 420.f;
                    ballZ  = 22.f;
                    ballVz = 240.f;
                    curveBounceActive = false;
                    curveBouncePhase  = 0.f;
                    ballSpin    = (dirX != 0.f) ? dirX * 260.f : 0.f;
                    curveActive = false;
                    curvePassed = false;
                    ballInPlay  = true;
                    serving     = false;
                    serveText.setString("");
                }

                // P1 DASH
                if (key->code == sf::Keyboard::Key::Q &&
                    !p1Dashing && ballInPlay)
                {
                    p1Dashing   = true;
                    p1DashTimer = dashTime;
                    p1DashDirX  = 0.f; p1DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) p1DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) p1DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) p1DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) p1DashDirY =  1.f;
                    if (p1DashDirX == 0.f && p1DashDirY == 0.f) p1DashDirY = -1.f;
                }

                // P2 DASH
                if (key->code == sf::Keyboard::Key::RShift &&
                    !p2Dashing && ballInPlay)
                {
                    p2Dashing   = true;
                    p2DashTimer = dashTime;
                    p2DashDirX  = 0.f; p2DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  p2DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) p2DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    p2DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  p2DashDirY =  1.f;
                    if (p2DashDirX == 0.f && p2DashDirY == 0.f) p2DashDirY = 1.f;
                }

                // P1 SWING
                if (key->code == sf::Keyboard::Key::Z && !p1Swinging)
                {
                    p1Swinging   = true;
                    p1SwingTimer = swingDuration;
                }

                // P2 SWING
                if (key->code == sf::Keyboard::Key::Slash && !p2Swinging)
                {
                    p2Swinging   = true;
                    p2SwingTimer = swingDuration;
                }
            }
        }

        // P1 MOVEMENT
        if (!p1Dashing)
        {
            float vx = 0.f, vy = 0.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vx = -playerSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vx =  playerSpeed;
            if (!ballOwner || ballInPlay)
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vy = -playerSpeed;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vy =  playerSpeed;
            }
            p1.move({vx * dt, vy * dt});
        }
        else
        {
            p1DashTimer -= dt;
            p1.move({p1DashDirX * dashSpeed * dt, p1DashDirY * dashSpeed * dt});
            if (p1DashTimer <= 0.f) p1Dashing = false;
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
            pos1.x = clamp(pos1.x, courtOffsetX + 205.f, courtOffsetX + 385.f);
            pos1.y = clamp(pos1.y, courtOffsetY + 360.f, courtOffsetY + 585.f);
        }
        else
        {
            pos1.x = clamp(pos1.x, fieldLeft, fieldRight);
            pos1.y = clamp(pos1.y, courtOffsetY + 300.f, deadZoneBottom - 20.f);
        }
        p1.setPosition(pos1);

        auto pos2 = p2.getPosition();
        if (!ballInPlay && !ballOwner)
        {
            pos2.x = clamp(pos2.x, courtOffsetX + 15.f, courtOffsetX + 195.f);
            pos2.y = clamp(pos2.y, courtOffsetY + 15.f, courtOffsetY + 230.f);
        }
        else
        {
            pos2.x = clamp(pos2.x, fieldLeft, fieldRight);
            pos2.y = clamp(pos2.y, deadZoneTop + 20.f, courtOffsetY + 300.f);
        }
        p2.setPosition(pos2);

        // SWING TIMERS
        if (p1Swinging) { p1SwingTimer -= dt; if (p1SwingTimer <= 0.f) p1Swinging = false; }
        if (p2Swinging) { p2SwingTimer -= dt; if (p2SwingTimer <= 0.f) p2Swinging = false; }

        // BALL FOLLOW SERVER
        if (!ballInPlay)
        {
            ballZ    = 0.f;
            ballVz   = 0.f;
            ballSpin = 0.f;
            ballSprite.setRotation(sf::degrees(0.f));
            if (ballOwner)
                ball.setPosition({p1.getPosition().x, p1.getPosition().y - 30.f});
            else
                ball.setPosition({p2.getPosition().x, p2.getPosition().y + 30.f});
        }

        // BALL PHYSICS
        if (ballInPlay)
        {
         

            auto bpos = ball.getPosition();

            // CURVE SYSTEM
            if (curveActive)
            {
                bool insideCourt = bpos.x >= courtLeftEdge && bpos.x <= courtRightEdge;
                bool reached = (ballVy < 0.f && bpos.y <= curveTargetY) ||
                               (ballVy > 0.f && bpos.y >= curveTargetY);

                if (!curvePassed && reached)
                {
                    if (insideCourt)
                    {
                        ballVx += curveForce * dt * 30.f;
                        ballVx = clamp(ballVx, -320.f, 320.f);

                        if (abs(ballVx) >= abs(curveForce) * 0.75f)
                        {
                            ballVx = curveForce;
                            curvePassed       = true;
                            curveActive       = false;
                            curveBounceActive = true;
                            curveBouncePhase  = -3.14159265f / 2.f;

                            // DASH SWING — sharp direction change
                            if (abs(curveForce) >= 180.f)
                            {
                                ballVy *= 1.4f;
                                ballVx *= 1.1f;
                                ballVx = clamp(ballVx, -500.f, 500.f);
                                ballSpin *= 2.2f;
                                curveBouncePhase = -3.14159265f;
                            }
                        }
                    }
                    else
                    {
                        curveActive = false;
                        curvePassed = true;
                        ballVx      = 0.f;
                    }
                }
            }

            // Edge slowdown
            if (bpos.x < fieldLeft + 50.f && ballVx < 0.f)
                ballVx += 60.f * dt;
            if (bpos.x > fieldRight - 50.f && ballVx > 0.f)
                ballVx -= 60.f * dt;

            // Ball height simulation
            ballVz -= ballGravityZ * dt;
            ballZ  += ballVz * dt;
            if (ballZ < 0.f) { ballZ = 0.f; ballVz = 0.f; }

            ballSpin -= ballSpin * dt * 1.4f;
            if (abs(ballSpin) < 1.5f) ballSpin = 0.f;
            if (ballSpin == 0.f)
                ballSpin = ballVx * 0.12f + ballVy * 0.10f;

            if (curveBounceActive)
            {
                curveBouncePhase += dt * 8.f;
                if (curveBouncePhase >= 3.14159265f)
                {
                    curveBouncePhase  = 3.14159265f;
                    curveBounceActive = false;
                }
            }

            // Speed cap
            ballVx = clamp(ballVx, -550.f, 550.f);
            ballVy = clamp(ballVy, -600.f, 600.f);

            float nextX = clamp(bpos.x + ballVx * dt, fieldLeft, fieldRight);
            float nextY = bpos.y + ballVy * dt;
            ball.setPosition(sf::Vector2f(nextX, nextY));
            bpos = ball.getPosition();

            // SCORING
            if (bpos.y < deadZoneTop)
            {
                score1++;
                score1Text.setString("P1: " + to_string(score1));
                ballInPlay  = false;
                ballOwner   = false;
                serving     = true;
                curveActive = false;
                p2.setPosition({courtOffsetX + 100.f, courtOffsetY + 20.f});
                serveText.setString(". to Serve (P2)");
                serveText.setPosition({courtOffsetX + 80.f, courtOffsetY - 80.f});
            }

            if (bpos.y > deadZoneBottom)
            {
                score2++;
                score2Text.setString("P2: " + to_string(score2));
                ballInPlay  = false;
                ballOwner   = true;
                serving     = true;
                curveActive = false;
                p1.setPosition({courtOffsetX + 295.f, courtOffsetY + 580.f});
                serveText.setString("X to Serve (P1)");
                serveText.setPosition({courtOffsetX + 80.f, courtOffsetY + 630.f});
            }

            // HITBOXES
            sf::FloatRect ballB = ball.getGlobalBounds();

            sf::FloatRect p1Hit = p1.getGlobalBounds();
            p1Hit.position.x -= 20.f; p1Hit.size.x += 40.f;
            p1Hit.position.y -= 20.f; p1Hit.size.y += 40.f;

            sf::FloatRect p2Hit = p2.getGlobalBounds();
            p2Hit.position.x -= 20.f; p2Hit.size.x += 40.f;
            p2Hit.position.y -= 20.f; p2Hit.size.y += 40.f;

            // P1 HIT
            bool p1CanHit = (p1Swinging || p1Dashing) &&
                             p1HitCooldown <= 0.f &&
                             ball.getPosition().y > courtOffsetY + 300.f;

            if (p1CanHit && ballB.findIntersection(p1Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;
                hitToP2Field(dirX, p1Dashing);
                p1Swinging    = false;
                p1HitCooldown = hitCooldown;
            }

            // P2 HIT
            bool p2CanHit = (p2Swinging || p2Dashing) &&
                             p2HitCooldown <= 0.f &&
                             ball.getPosition().y < courtOffsetY + 300.f;

            if (p2CanHit && ballB.findIntersection(p2Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;
                hitToP1Field(dirX, p2Dashing);
                p2Swinging    = false;
                p2HitCooldown = hitCooldown;
            }
        }

        // DRAW
        window.clear();
        window.draw(backgroundSprite);
        window.draw(field);
        window.draw(court);
        window.draw(midLine);
        window.draw(netLine);

        if (p1Swinging) { p1SwingCircle.setPosition(p1.getPosition()); window.draw(p1SwingCircle); }
        if (p2Swinging) { p2SwingCircle.setPosition(p2.getPosition()); window.draw(p2SwingCircle); }

        window.draw(p1);
        window.draw(p2);

        float heightRatio = clamp(ballZ / 120.f, 0.f, 1.f);
        float spriteScale = 0.8f + heightRatio * 0.18f;
        if (curveBounceActive)
        {
            float bounceAmount = sin(curveBouncePhase);
            spriteScale *= 1.f + 0.12f * bounceAmount;
        }
        ballSprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        ballSprite.rotate(sf::degrees(ballSpin * dt));
        ballSprite.setPosition(sf::Vector2f(ball.getPosition().x,
                                             ball.getPosition().y - ballZ * 0.6f));

        float shadowScale = clamp(1.f - heightRatio * 0.25f, 0.7f, 1.f);
        ballShadow.setScale(sf::Vector2f(shadowScale, shadowScale * 0.45f));
        ballShadow.setPosition(sf::Vector2f(ball.getPosition().x,
                                             ball.getPosition().y + 6.f));

        window.draw(ballShadow);
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
}
