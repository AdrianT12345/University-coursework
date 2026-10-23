<?php
session_start();
require_once 'functions.php';
if (isset($_SESSION['user_id'])) { header("Location: index.php"); exit; }

$err = '';
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $user = verifyLogin($_POST['login'], $_POST['password']);
    if ($user) {
        $_SESSION['user_id'] = (string)$user['_id'];
        $_SESSION['user_login'] = $user['login'];
        $_SESSION['user_photo'] = $user['profile_photo'];
        header("Location: index.php"); exit;
    } else {
        $err = "Błędny login lub hasło.";
    }
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Logowanie</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <?php include 'menu_partial.php'; ?>
    <main class="container" style="max-width: 400px;">
        <header><h2>Logowanie</h2></header>
        <?php if ($err): ?><div class="message error"><?= $err ?></div><?php endif; ?>
        <?php if (isset($_SESSION['flash'])): ?><div class="message success"><?= $_SESSION['flash']; unset($_SESSION['flash']); ?></div><?php endif; ?>
        <form method="POST">
            <div class="form-group"><label>Login:</label><input type="text" name="login" required></div>
            <div class="form-group"><label>Hasło:</label><input type="password" name="password" required></div>
            <button type="submit" class="btn-primary">Zaloguj</button>
        </form>
    </main>
</body>
</html>