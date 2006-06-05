<?php
# Test to see that BILE handles can handle content that appears before the 
# HTML element
include('globals.inc');
?>
<html>
<head>
<title>A PHP page</title>
</head>
<body>
	<table>
		<thead>
			<tr>
				<th>Col 1</th>
				<th>Col 2</th>
			</tr>
		</thead>
		<tbody>
			<?php while($rec = @mysql_query($result)): ?>
			<tr>
				<td><?php echo htmlspecialchars($rec['col1']); ?></td>
				<td><?php echo htmlspecialchars($rec['col2']); ?></td>
			</tr>
			<?php endwhile; ?>
		</tbody>
	</table>
</body>
</html>