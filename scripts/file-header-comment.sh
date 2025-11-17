#!/usr/bin/env bash
set -euo pipefail

AUTHOR_DEFAULT="Maurice Green"
COMPANY_DEFAULT="Trace Systems, LLC"
BRIEF_DEFAULT="Main application entry point and core logic."

usage() {
  echo "Usage: $(basename "$0") [-b \"brief text\"] [-a \"Author\"] [-y YEAR] file"
  exit 1
}

BRIEF="$BRIEF_DEFAULT"
AUTHOR="$AUTHOR_DEFAULT"
YEAR=""
while getopts ":b:a:y:" opt; do
  case "$opt" in
    b) BRIEF="$OPTARG" ;;
    a) AUTHOR="$OPTARG" ;;
    y) YEAR="$OPTARG" ;;
    *) usage ;;
  esac
done
shift $((OPTIND -1))

[[ $# -eq 1 ]] || usage
FILE="$1"

# Compute fields
DATE="$(date +"%B %d, %Y")"
YEAR="${YEAR:-$(date +%Y)}"
FILENAME="$(basename "$FILE")"

# If header already exists, bail fast
if [[ -f "$FILE" ]]; then
  if head -n 1 "$FILE" | grep -q "/\\*\\*\\*"; then
    if head -n 10 "$FILE" | grep -q "@file"; then
      echo "Header already present in $FILE; skipping." >&2
      exit 0
    fi
  fi
fi

# Build header in a temp file
TMPHDR="$(mktemp)"
cat > "$TMPHDR" <<EOF
/*******************************************************************************
 * @file               $FILENAME
 * @brief              $BRIEF
 * @author             $AUTHOR
 * @date               $DATE
 * @copyright          (C) $YEAR $COMPANY_DEFAULT.  All rights reserved.
 *
 * @details            This line will provide details about the actions performed
 *                     by the functions in the file
 *
 * @revision           $DATE - $AUTHOR - init
 ******************************************************************************/
EOF

# Prepend: create file if missing, otherwise concatenate
TMPALL="$(mktemp)"
if [[ -f "$FILE" ]]; then
  cat "$TMPHDR" "$FILE" > "$TMPALL"
else
  cat "$TMPHDR" > "$TMPALL"
fi

# Preserve original permissions if file existed
if [[ -f "$FILE" ]]; then
  perms=$(stat -c %a "$FILE" 2>/dev/null || stat -f %Lp "$FILE")
  mv "$TMPALL" "$FILE"
  chmod "$perms" "$FILE" || true
else
  mv "$TMPALL" "$FILE"
fi

rm -f "$TMPHDR"
echo "Header added to $FILE"
