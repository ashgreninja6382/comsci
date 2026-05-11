#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode({600, 800}), "Super Pickleball Adventure");
    window.setFramerateLimit(60);

    // Green open field background
    sf::RectangleShape field({600.f, 800.f});
    field.setFillColor(sf::Color(34, 100, 34));
    field.setPosition({0.f, 0.f});

    // Court (visual only)
    sf::RectangleShape court({400.f, 600.f});
    court.setFillColor(sf::Color(150, 180, 150));
    court.setOutlineColor(sf::Color::White);
    court.setOutlineThickness(3.f);
    court.setPosition({100.f, 100.f});

    sf::RectangleShape netLine({400.f, 10.f});
    netLine.setFillColor(sf::Color::White);
    netLine.setPosition({100.f, 395.f});

    sf::RectangleShape midLine({3.f, 600.f});
    midLine.setFillColor(sf::Color::White);
    midLine.setPosition({300.f, 100.f});

    // Players
    sf::RectangleShape p1({30.f, 40.f});
    p1.setFillColor(sf::Color::Red);
    p1.setOrigin({15.f, 20.f});
    p1.setPosition({395.f, 680.f});

    sf::RectangleShape p2({30.f, 40.f});
    p2.setFillColor(sf::Color(128, 0, 200));
    p2.setOrigin({15.f, 20.f});
    p2.setPosition({200.f, 120.f});

    // Ball
    sf::CircleShape ball(12.f);
    ball.setFillColor(sf::Color::White);
    ball.setOutlineColor(sf::Color::Black);
    ball.setOutlineThickness(2.f);
    ball.setOrigin({12.f, 12.f});
    ball.setPosition({395.f, 650.f});

    float ballVx = 0.f, ballVy = 0.f;

// Gentle curve system
float curveAmount = 0.f;
float curveTargetY = 0.f;
bool curveActive = false;

    bool ballInPlay = false;
    bool ballOwner = true;
    bool serving = true;

    const float playerSpeed = 250.f;
    const float dashSpeed   = 700.f;
    const float dashTime    = 0.15f;

    bool  p1Dashing = false, p2Dashing = false;
    float p1DashTimer = 0.f,  p2DashTimer = 0.f;
    float p1DashDirX = 0.f,   p1DashDirY = 0.f;
    float p2DashDirX = 0.f,   p2DashDirY = 0.f;

    bool  p1Swinging = false, p2Swinging = false;
    float p1SwingTimer = 0.f, p2SwingTimer = 0.f;
    const float swingDuration = 0.25f;

    float p1HitCooldown = 0.f;
    float p2HitCooldown = 0.f;
    const float hitCooldown = 0.3f;

    // Dead zones — only here does a point get scored
    const float deadZoneTop    = 20.f;
    const float deadZoneBottom = 780.f;

    // Open field bounds — ball and players can roam here freely
    const float fieldLeft  = 20.f;
    const float fieldRight = 580.f;

    int score1 = 0, score2 = 0;
    sf::Font font;
    font.openFromFile("C:/Windows/Fonts/arial.ttf");

    sf::Text score1Text(font, "P1: 0", 28);
    score1Text.setFillColor(sf::Color::White);
    score1Text.setPosition({20.f, 730.f});

    sf::Text score2Text(font, "P2: 0", 28);
    score2Text.setFillColor(sf::Color::White);
    score2Text.setPosition({420.f, 20.f});

    sf::Text serveText(font, "X to Serve (P1)", 20);
    serveText.setFillColor(sf::Color::Yellow);
    serveText.setPosition({180.f, 760.f});

    sf::CircleShape p1SwingCircle(35.f);
    p1SwingCircle.setFillColor(sf::Color(255, 255, 0, 120));
    p1SwingCircle.setOrigin({35.f, 35.f});

    sf::CircleShape p2SwingCircle(35.f);
    p2SwingCircle.setFillColor(sf::Color(0, 200, 255, 120));
    p2SwingCircle.setOrigin({35.f, 35.f});

       // Hit helpers — with soft edge correction system
    // When near edges, gracefully guides the ball instead of reversing direction
   auto hitToP2Field = [&](float dirX, bool dashing)
{
    float power = dashing ? 560.f : 420.f;
    auto ballPos = ball.getPosition();

    const float courtLeft = 120.f;
    const float courtRight = 480.f;

    float timeToReach = 300.f / power;

    // Very small initial drift
    float targetX = ballPos.x + dirX * 35.f;

    // Soft edge correction
    if (targetX < courtLeft + 20.f)
        targetX = courtLeft + 20.f;

    if (targetX > courtRight - 20.f)
        targetX = courtRight - 20.f;

    ballVx = (targetX - ballPos.x) / timeToReach;

    ballVy = -power;

    // Very gentle curve setup
    if (dirX != 0.f)
    {
        curveAmount = dirX * 22.f;
        curveTargetY = 275.f;
        curveActive = true;
    }
    else
    {
        curveActive = false;
    }

    ballInPlay = true;
};

    auto hitToP1Field = [&](float dirX, bool dashing)
{
    float power = dashing ? 560.f : 420.f;
    auto ballPos = ball.getPosition();

    const float courtLeft = 120.f;
    const float courtRight = 480.f;

    float timeToReach = 300.f / power;

    // Very small initial drift
    float targetX = ballPos.x + dirX * 35.f;

    // Soft edge correction
    if (targetX < courtLeft + 20.f)
        targetX = courtLeft + 20.f;

    if (targetX > courtRight - 20.f)
        targetX = courtRight - 20.f;

    ballVx = (targetX - ballPos.x) / timeToReach;

    ballVy = power;

    // Very gentle curve setup
    if (dirX != 0.f)
    {
        curveAmount = dirX * 22.f;
        curveTargetY = 525.f;
        curveActive = true;
    }
    else
    {
        curveActive = false;
    }

    ballInPlay = true;
};

    sf::Text p1PosText(font, "P1 (0, 0)", 16);
    p1PosText.setFillColor(sf::Color::Yellow);
    p1PosText.setPosition({10.f, 50.f});

    sf::Text p2PosText(font, "P2 (0, 0)", 16);
    p2PosText.setFillColor(sf::Color::Cyan);
    p2PosText.setPosition({10.f, 20.f});

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        if (p1HitCooldown > 0.f) p1HitCooldown -= dt;
        if (p2HitCooldown > 0.f) p2HitCooldown -= dt;

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                // P1 serve
                if (key->code == sf::Keyboard::Key::X && !ballInPlay && ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;
                    ballInPlay = true;
                    serving = false;
                    ballVx = dirX * 80.f;
                    ballVy = -420.f;
                    serveText.setString("");
                }

                // P2 serve
                if (key->code == sf::Keyboard::Key::Period && !ballInPlay && !ballOwner)
                {
                    float dirX = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;
                    ballInPlay = true;
                    serving = false;
                    ballVx = dirX * 80.f;
                    ballVy = 420.f;
                    serveText.setString("");
                }

                // P1 dash (Q) — disabled during serve
                if (key->code == sf::Keyboard::Key::Q && !p1Dashing && ballInPlay)
                {
                    p1Dashing = true;
                    p1DashTimer = dashTime;
                    p1DashDirX = 0.f; p1DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) p1DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) p1DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) p1DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) p1DashDirY =  1.f;
                    if (p1DashDirX == 0.f && p1DashDirY == 0.f) p1DashDirY = -1.f;
                }

                // P2 dash (RShift) — disabled during serve
                if (key->code == sf::Keyboard::Key::RShift && !p2Dashing && ballInPlay)
                {
                    p2Dashing = true;
                    p2DashTimer = dashTime;
                    p2DashDirX = 0.f; p2DashDirY = 0.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  p2DashDirX = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) p2DashDirX =  1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    p2DashDirY = -1.f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  p2DashDirY =  1.f;
                    if (p2DashDirX == 0.f && p2DashDirY == 0.f) p2DashDirY = 1.f;
                }

                // P1 swing (Z)
                if (key->code == sf::Keyboard::Key::Z && !p1Swinging)
                {
                    p1Swinging = true;
                    p1SwingTimer = swingDuration;
                }

                // P2 swing (Slash)
                if (key->code == sf::Keyboard::Key::Slash && !p2Swinging)
                {
                    p2Swinging = true;
                    p2SwingTimer = swingDuration;
                }
            }
        }

        // P1 movement — open field bottom area
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

        // P2 movement — open field top area
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

        // Clamp players to open field
        auto pos1 = p1.getPosition();
        if (!ballInPlay && ballOwner)
        {
            // P1 serving zone: lower right
            pos1.x = clamp(pos1.x, 305.f, 485.f);
            pos1.y = clamp(pos1.y, 460.f, 685.f);
        }
        else
        {
            // P1 open field: bottom half of full window
            pos1.x = clamp(pos1.x, fieldLeft, fieldRight);
            pos1.y = clamp(pos1.y, 400.f, deadZoneBottom - 20.f);
        }
        p1.setPosition(pos1);

        auto pos2 = p2.getPosition();
        if (!ballInPlay && !ballOwner)
        {
            // P2 serving zone: upper left
            pos2.x = clamp(pos2.x, 115.f, 295.f);
            pos2.y = clamp(pos2.y, 115.f, 330.f);
        }
        else
        {
            // P2 open field: top half of full window
            pos2.x = clamp(pos2.x, fieldLeft, fieldRight);
            pos2.y = clamp(pos2.y, deadZoneTop + 20.f, 400.f);
        }
        p2.setPosition(pos2);

        // Swing timers
        if (p1Swinging) { p1SwingTimer -= dt; if (p1SwingTimer <= 0.f) p1Swinging = false; }
        if (p2Swinging) { p2SwingTimer -= dt; if (p2SwingTimer <= 0.f) p2Swinging = false; }

        // Ball follows server
        if (!ballInPlay)
        {
            if (ballOwner) ball.setPosition({p1.getPosition().x, p1.getPosition().y - 30.f});
            else           ball.setPosition({p2.getPosition().x, p2.getPosition().y + 30.f});
        }

        // Ball physics
        if (ballInPlay)
        {
        

            // Slow down horizontal near open field edges
            auto bpos = ball.getPosition();
            if (bpos.x < fieldLeft + 30.f && ballVx < 0.f) ballVx *= 0.85f;
            if (bpos.x > fieldRight - 30.f && ballVx > 0.f) ballVx *= 0.85f;

            // Cap speed
            float maxSpeed = 600.f;
            if (abs(ballVx) > maxSpeed) ballVx = (ballVx > 0 ? 1.f : -1.f) * maxSpeed;
            if (abs(ballVy) > maxSpeed) ballVy = (ballVy > 0 ? 1.f : -1.f) * maxSpeed;

            // Move ball — clamp only to open field left/right, never bounce
            float nextX = clamp(bpos.x + ballVx * dt, fieldLeft, fieldRight);
            float nextY = bpos.y + ballVy * dt;
            ball.setPosition({nextX, nextY});
            bpos = ball.getPosition();

            // Gentle curve change
if (curveActive)
{
    // P1 upward shot
    if (ballVy < 0.f && bpos.y <= curveTargetY)
    {
        ballVx += curveAmount;

        // Small slowdown after curve
        ballVy *= 0.96f;

        curveActive = false;
    }

    // P2 downward shot
    else if (ballVy > 0.f && bpos.y >= curveTargetY)
    {
        ballVx += curveAmount;

        // Small slowdown after curve
        ballVy *= 0.96f;

        curveActive = false;
    }
}

            // Only score at dead zones (very top or very bottom)
            if (bpos.y < deadZoneTop)
            {
                score1++;
                score1Text.setString("P1: " + to_string(score1));
                ballInPlay = false;
                ballOwner = false;
                serving = true;
                p2.setPosition({200.f, 120.f});
                serveText.setString(". to Serve (P2)");
                serveText.setPosition({190.f, 20.f});
            }

            if (bpos.y > deadZoneBottom)
            {
                score2++;
                score2Text.setString("P2: " + to_string(score2));
                ballInPlay = false;
                ballOwner = true;
                serving = true;
                p1.setPosition({395.f, 680.f});
                serveText.setString("X to Serve (P1)");
                serveText.setPosition({180.f, 760.f});
            }

            // Hit detection — players can hit from anywhere in their half
            sf::FloatRect ballB = ball.getGlobalBounds();

            sf::FloatRect p1Hit = p1.getGlobalBounds();
            p1Hit.position.x -= 20.f; p1Hit.size.x += 40.f;
            p1Hit.position.y -= 20.f; p1Hit.size.y += 40.f;

            sf::FloatRect p2Hit = p2.getGlobalBounds();
            p2Hit.position.x -= 20.f; p2Hit.size.x += 40.f;
            p2Hit.position.y -= 20.f; p2Hit.size.y += 40.f;

            // P1 hits when ball is in bottom half of window
            bool p1CanHit = (p1Swinging || p1Dashing)
                            && p1HitCooldown <= 0.f
                            && ball.getPosition().y > 400.f;

            if (p1CanHit && ballB.findIntersection(p1Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dirX =  1.f;
                hitToP2Field(dirX, p1Dashing);
                p1Swinging = false;
                p1HitCooldown = hitCooldown;
            }

            // P2 hits when ball is in top half of window
            bool p2CanHit = (p2Swinging || p2Dashing)
                            && p2HitCooldown <= 0.f
                            && ball.getPosition().y < 400.f;

            if (p2CanHit && ballB.findIntersection(p2Hit))
            {
                float dirX = 0.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dirX = -1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dirX =  1.f;
                hitToP1Field(dirX, p2Dashing);
                p2Swinging = false;
                p2HitCooldown = hitCooldown;
            }
        }

        // Draw
        window.clear();
        window.draw(field);       // green open field background
        window.draw(court);       // white court lines (visual only)
        window.draw(midLine);
        window.draw(netLine);

        if (p1Swinging) { p1SwingCircle.setPosition(p1.getPosition()); window.draw(p1SwingCircle); }
        if (p2Swinging) { p2SwingCircle.setPosition(p2.getPosition()); window.draw(p2SwingCircle); }

        window.draw(p1);
        window.draw(p2);
        window.draw(ball);
        window.draw(score1Text);
        window.draw(score2Text);
        window.draw(serveText);

        p1PosText.setString("P1 (" + to_string((int)p1.getPosition().x) + ", " + to_string((int)p1.getPosition().y) + ")");
        p2PosText.setString("P2 (" + to_string((int)p2.getPosition().x) + ", " + to_string((int)p2.getPosition().y) + ")");
        window.draw(p1PosText);
        window.draw(p2PosText);

        window.display();
    }
}
