#!/usr/bin/env bash
# ============================================================================
# tag_and_push.sh — bump de VERSION según el último tag, tag + push.
#
# Cumple la regla de docs/LEARNINGS.md: TODO push lleva su tag y VERSION
# siempre coincide con el último tag. Corre al final de cada cambio y hace:
#   1) calcula la siguiente versión desde el último tag (ciclo patch 0-9:
#      x.y.9 -> x.(y+1).0)
#   2) opcionalmente commitea los cambios pendientes (modificados + agregados)
#   3) bumpea VERSION, commitea el bump, crea el tag y pushea main + tag
#
# Uso:
#   ./scripts/tag_and_push.sh                      # sin cambios pendientes
#   ./scripts/tag_and_push.sh "feat: mensaje"      # commitea lo pendiente y sigue
#
# Nota: los archivos NUEVOS deben agregarse antes (git add <archivo>) para
# que entren en el commit; los que queden sin trackear solo se listan como
# aviso y no bloquean.
# ============================================================================
set -euo pipefail

cd "$(dirname "$0")/.."

# --- 1) Último tag publicado -------------------------------------------------
LAST_TAG="$(git describe --tags --abbrev=0 2>/dev/null || echo v0.0.0)"
LAST_VERSION="${LAST_TAG#v}"

# --- 2) Siguiente versión: patch +1, y al llegar a 9 -> minor (ciclo 0-9) ----
IFS='.' read -r MAJ MIN PATCH <<< "$LAST_VERSION"
if (( PATCH < 9 )); then
    NEXT="${MAJ}.${MIN}.$((PATCH + 1))"
else
    NEXT="${MAJ}.$((MIN + 1)).0"
fi

echo "== Último tag: $LAST_TAG  ->  próxima versión: $NEXT"

# --- 3) Cambios pendientes ---------------------------------------------------
PENDING="$(git status --porcelain)"
if [ -n "$PENDING" ]; then
    if [ $# -ge 1 ]; then
        UNTRACKED="$(git ls-files --others --exclude-standard)"
        if [ -n "$UNTRACKED" ]; then
            echo "⚠  Archivos sin trackear (no entran en el commit):"
            echo "$UNTRACKED" | sed 's/^/    /'
        fi
        git add -u
        git commit -m "$1"
    else
        echo "⚠  Hay cambios sin commitear y no se pasó mensaje de commit."
        echo "   Pasalo como argumento (ej: ./scripts/tag_and_push.sh \"fix: ...\")"
        echo "   o commité a mano antes de volver a correr el script."
        exit 1
    fi
fi

# --- 4) Bump + tag + push ----------------------------------------------------
echo "$NEXT" > VERSION
git add VERSION
git commit -m "chore: bump VERSION to $NEXT"
git tag "v$NEXT"
git push origin main
git push origin "v$NEXT"

echo "✅ v$NEXT pusheado (VERSION=$NEXT)."
