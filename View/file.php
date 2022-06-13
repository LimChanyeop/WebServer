<!DOCTYPE html>
<html lang="ko">
    <head>
        <meta charset="utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no" />
        <meta name="description" content="" />
        <meta name="author" content="" />
        <title>File Manager</title>
        <style>
            ol li {
                list-style: none;
                padding: 5px 0px 5px 5px;
                margin-bottom: 5px;
                border-bottom: 1px solid #354251;
                font-size: 12px;
            }
            ol li:before {
                list-style: none;
                content: ">";
                display: inline-block;
                vertical-align: middle;
                padding: 0px 5px 6px 0px;
            }
        </style>
    </head>
    <body>
<?php

// 디렉토리 설정
$upload_dir = "/uploads";
//$dir = $_SERVER['DOCUMENT_ROOT'] . $upload_dir;
$dir = getcwd() . $upload_dir;
if (is_dir($dir) == false) {
    mkdir($dir);
}

if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
    $target_file = $dir . '/' . $_GET['filename'];
    if (is_file($target_file)) {
        if(!unlink($target_file)) {
            echo $_GET['filename'] . " is not deleted\n";
        } else {
            echo $_GET['filename'] . " is deleted\n";
        }
    } else {
        echo $_GET['filename'] . " is not Exist\n";
    }
    echo "<br />";
} else if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo '
    <form action="file.php" method="post" enctype="multipart/form-data">
      Select image to upload:
      <input type="file" name="fileToUpload" id="fileToUpload">
      <input type="submit" value="Upload Image" name="submit">
    </form>';
    echo "<br />";
} else if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $upload_file = $dir . '/' . basename($_FILES["fileToUpload"]["name"]);
    $imageFileType = strtolower(pathinfo($upload_file, PATHINFO_EXTENSION));
    $uploadOk = 1;

    // Check if image file is a actual image or fake image
    if(isset($_POST["submit"])) {
      $check = getimagesize($_FILES["fileToUpload"]["tmp_name"]);
      if($check !== false) {
        echo "File is an image - " . $check["mime"] . ".";
        $uploadOk = 1;
      } else {
        echo "File is not an image.";
        $uploadOk = 0;
      }
    }

    // Check if file already exists
    if (file_exists($upload_file)) {
      echo "Sorry, file already exists.";
      $uploadOk = 0;
    }
    
    // Check file size
    if ($_FILES["fileToUpload"]["size"] > 3000000) {
      echo "Sorry, your file is too large.";
      $uploadOk = 0;
    }
    
    // Allow certain file formats
    if($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg"
    && $imageFileType != "gif") {
      echo "Sorry, only JPG, JPEG, PNG & GIF files are allowed.";
      $uploadOk = 0;
    }
    
    // Check if $uploadOk is set to 0 by an error
    if ($uploadOk == 0) {
      echo "Sorry, your file was not uploaded.";
    // if everything is ok, try to upload file
    } else {
      if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $upload_file)) {
        echo "The file ". htmlspecialchars( basename( $_FILES["fileToUpload"]["name"])). " has been uploaded.";
      } else {
        echo "Sorry, there was an error uploading your file.";
      }
    }
    echo "<br />";
}

// 핸들 설정
$handle = opendir($dir);

$files = array();

// 디렉토리에 포함된 파일을 배열에 추가
while (($filename = readdir($handle)) !== false) {
    if ($filename == '.' || $filename == '..') {
        continue;
    }

    // 파일인 경우에만 추가
    if (is_file($dir . "/" . $filename)) {
        $files[] = $filename;
    }
}

// 핸들 해제
closedir($handle);

?>
<ol>
<?php
// 파일명 출력
foreach ($files as $f) {
    echo "<li><a href=\"." . $upload_dir . '/' . $f . "\">" . $f . "</a></li>";
    echo "<br />";
}
?>
</ol>
    </body>
<html>