<nav class="navbar">
    <div style="font-size: 1.2rem; font-weight: bold;">
        <a href="index.php" style="margin:0;">Galeria App</a>
    </div>
    
    <div style="display:flex; align-items:center;">
        
        <?php include 'cart_partial.php'; ?>

        <div class="user-info">
            <?php if (isset($_SESSION['user_id'])): ?>
                <span>Witaj, <b><?= htmlspecialchars($_SESSION['user_login']) ?></b></span>
                <?php if (!empty($_SESSION['user_photo'])): ?>
                    <img src="<?= PROFILE_DIR . $_SESSION['user_photo'] ?>" alt="foto" class="profile-mini">
                <?php endif; ?>
                <a href="logout.php" class="logout-btn">Wyloguj</a>
            <?php else: ?>
                <a href="login.php">Logowanie</a>
                <a href="register.php">Rejestracja</a>
            <?php endif; ?>
        </div>
    </div>
</nav>