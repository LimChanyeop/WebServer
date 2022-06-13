<!DOCTYPE html>
<html>
<body>

<form method="post" action="<?php echo $_SERVER["PHP_SELF"];?>">
  id:   <input type="text" name="fid"> <br>
  Name: <input type="text" name="fname">
  <input type="submit">
</form>

<?php
$method = $_SERVER["REQUEST_METHOD"];
$phpself = $_SERVER["PHP_SELF"];
echo "PHP_SELF: $phpself<br>";
echo "Method: $method <br>";
if ($_SERVER["REQUEST_METHOD"] == "POST") {
  $name = $_POST['fname'];
  if (empty($name)) {
    echo "Name is empty";
  } else {
    echo "[name]: $name";
  }
  echo "<br>";
  $fid = $_POST['fid'];
  if (empty($fid)) {
    echo "id is empty";
  } else {
    echo "[id]: $fid";
  }
}
?>

</body>
</html>
