# Start with your main binary
readelf -d /usr/local/bin/sdrberry | grep NEEDED

# Recursively check all its dependencies
ldd /usr/local/bin/sdrberry | awk '/=>/ {print $3}' | while read lib; do
  echo "=== $lib ==="
  readelf -d "$lib" 2>/dev/null | grep -E "(NEEDED|SONAME)"
done
