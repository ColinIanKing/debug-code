BEGIN { i = 0; v = 0 }
{ v = v + $8;
  if (i == 0) p = $5
  i = i + 1;
  if (i == N) {
		print substr(p, 2)+0 " " v / N;
		v = 0;
		i = 0;
	}

}
