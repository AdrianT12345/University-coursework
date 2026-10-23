<?php
require_once __DIR__ . '/vendor/autoload.php';
// konfiguracja folderow
define('UPLOAD_DIR', './images/');
define('PROFILE_DIR', './ProfilesFoto/');
define('ALLOWED_TYPES', ['jpg', 'jpeg', 'png']);

// ustawienia bazy
define('DB_NAME', 'gallery_db');
define('DB_COLLECTION', 'photos');
define('DB_USERS', 'users');

// polaczenie z mongodb
function getDBClient() {
    try {
        return new MongoDB\Client("mongodb://student:student@localhost:27017/gallery_db");
    } catch (Exception $e) {
        die("Problem z baza: " . $e->getMessage());
    }
}

// pobieranie kolekcji
function getGalleryCollection() { return getDBClient()->selectCollection(DB_NAME, DB_COLLECTION); }
function getUsersCollection() { return getDBClient()->selectCollection(DB_NAME, DB_USERS); }

// rejestracja z hashowaniem hasla
function registerUser($email, $login, $password, $profilePhoto) {
    $db = getUsersCollection();
    $hash = password_hash($password, PASSWORD_DEFAULT);
    
    $db->insertOne([
        'email' => $email,
        'login' => $login,
        'password' => $hash,
        'profile_photo' => $profilePhoto,
        'created_at' => new MongoDB\BSON\UTCDateTime(time() * 1000)
    ]);
    return true;
}

// sprawdzanie czy login wolny
function isLoginTaken($login) {
    $user = getUsersCollection()->findOne(['login' => $login]);
    return $user !== null;
}

// logowanie
function verifyLogin($login, $password) {
    $user = getUsersCollection()->findOne(['login' => $login]);
    if ($user && password_verify($password, $user['password'])) {
        return $user;
    }
    return false;
}

// zapis zdjecia do bazy
function saveImageToDB($fileName, $thumbName, $title, $author, $userId = null, $privacy = 'public') {
    $db = getGalleryCollection();
    $db->insertOne([
        'file_name' => $fileName,
        'thumb_name' => $thumbName,
        'title'     => $title,
        'author'    => $author,
        'user_id'   => $userId,
        'privacy'   => $privacy,
        'uploaded_at' => new MongoDB\BSON\UTCDateTime(time() * 1000) 
    ]);
}

// filtr widocznosci (swoje + publiczne)
function getPrivacyFilter($currentUserId) {
    $warunki = [['privacy' => 'public']];
    if ($currentUserId) {
        $warunki[] = ['user_id' => $currentUserId];
    }
    return ['$or' => $warunki];
}

// liczenie ilosci zdjec (do stron)
function countImages($currentUserId = null) {
    return getGalleryCollection()->countDocuments(getPrivacyFilter($currentUserId));
}

// pobieranie zdjec na strone
function getGalleryImages($page, $perPage, $currentUserId = null) {
    $skip = ($page - 1) * $perPage;
    $opcje = ['sort' => ['uploaded_at' => -1], 'limit' => $perPage, 'skip' => $skip];
    return getGalleryCollection()->find(getPrivacyFilter($currentUserId), $opcje)->toArray();
}

// pobieranie po id (do koszyka)
function getImagesByIds($idArray) {
    $db = getGalleryCollection();
    $ids = [];
    foreach ($idArray as $id) {
        try { 
            $ids[] = new MongoDB\BSON\ObjectId($id); 
        } catch (Exception $e) { 
            continue; 
        }
    }
    return $db->find(['_id' => ['$in' => $ids]])->toArray();
}

// robienie miniatury
function createThumbnail($source, $dest, $ext) {
    $w = 200; 
    $h = 125;
    list($origW, $origH) = getimagesize($source);
    
    $thumb = imagecreatetruecolor($w, $h);
    $img = null;

    if ($ext == 'jpg' || $ext == 'jpeg') $img = imagecreatefromjpeg($source);
    elseif ($ext == 'png') {
        $img = imagecreatefrompng($source);
        imagealphablending($thumb, false); 
        imagesavealpha($thumb, true);
    }

    if (!$img) return false;
    
    imagecopyresampled($thumb, $img, 0, 0, 0, 0, $w, $h, $origW, $origH);

    if ($ext == 'jpg' || $ext == 'jpeg') imagejpeg($thumb, $dest, 90);
    elseif ($ext == 'png') imagepng($thumb, $dest);

    imagedestroy($thumb); 
    imagedestroy($img);
    return true;
}
?>