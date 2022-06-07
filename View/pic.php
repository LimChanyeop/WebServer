<!DOCTYPE html>
<head>
	<meta charset="utf-8">
	<link rel="stylesheet" type="text/css" href="http://localhost/style.css">
</head>
<header>
	<img src="https://velog.velcdn.com/images%2Fwuriae%2Fpost%2F9e9efa8c-7d6f-436b-b6aa-9be03b6e29e7%2FKakaoTalk_20210623_005843711_02.jpg" alt="minsikim">
	<h1><a href="http://localhost:4242/">go to home</a></h1>
</header>
<nav>
	<ol>
	<?php
		echo file_get_contents("list.txt");
	?>
		</ ol>
	</nav>
	<div id="control">
		<input type="button" value="white" onclick="document.getElementById('target').className='white'"/>
		<input type="button" value="black" onclick="document.getElementById('target').className='black'" />
	</div>
<article>
	<?php
		if( empty($_GET['id']) == false ) {
			echo file_get_contents($_GET['id'].".txt");
			}
	?>
</article>