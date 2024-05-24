
int32_t getData_(byte K, int32_t *sensor_error, byte N)
{
  int32_t d[N];
  int32_t d_avg = 0;
  int32_t d_worst = 0;
  int32_t d2 = 0;

  for (byte i = 0; i < N; i++)
  {
    d2 = sensor.getData();
    if (abs(d2) < Scale * MAX_LOAD)
    {
      d[i] = d2;
      d_avg += d2;
    }
    else
    {
      i--;
    }
  }

  d_avg /= N;
  sort_(d, N, d_avg);
  d_worst = abs(d[N - 1] - d_avg);

  int32_t d_prev = 0;
  int32_t distance = d_worst;
  byte count = 0;
  byte k = 0;
  while (k < K && d_worst > (int)(Scale * Absolute_error))
  {
    d2 = sensor.getData();
    if (abs(d2 - d_avg) < d_worst)
    {
      d_avg += (d2 - d[N - 1]) / N;
      d[N - 1] = d2;
      sort_(d, N, d_avg);
      d_worst = abs(d[N - 1] - d_avg);
    }
    k++;
    if (d2 - d_prev > distance)
      count++;
    else
      count = 0;
    d_prev = d2;
  }

  if (sensor_error != NULL)
  {
    *sensor_error = d_worst;
  }
  return d_avg;
}