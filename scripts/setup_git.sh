#!/usr/bin/env bash
#
# setup_git.sh - configura el repositorio Git del proyecto AHT21B_bh1750.
#
# Si ya existe un repo local (.git) lo detecta y solo ofrece añadir el remote
# y hacer push; si no existe, lo inicializa. En ambos casos pregunta los datos
# necesarios y usa `gh` si está disponible, con fallback a pasos manuales.
#
# Reglas del proyecto (docs/LEARNINGS.md): todo push lleva su tag y VERSION
# siempre coincide con el último tag.

set -euo pipefail

GREEN='\033[0;32m'; RED='\033[0;31m'; NC='\033[0m'
ok()  { echo -e "${GREEN}[git]${NC} $*"; }
die() { echo -e "${RED}[git]${NC} $*" >&2; exit 1; }

VERSION="$(cat VERSION 2>/dev/null || echo 0.0.0)"
REPO_NAME="$(basename "$(pwd)")"

# ---- 1. Inicializar el repo si hace falta -------------------------------
if [[ -d .git ]]; then
    ok "Repositorio ya inicializado; se conserva el historial."
else
    ok "Inicializando repositorio local..."
    git init -b main
    git add .
    git commit -m "feat: initial project scaffolding" || true
fi

# ---- 2. Recoger datos del usuario ----------------------------------------
read -rp "Usuario de GitHub/GitLab [$(git config user.name 2>/dev/null || echo '')]: " USERNAME
[[ -z "${USERNAME}" ]] && USERNAME="$(git config user.name 2>/dev/null || true)"
[[ -z "${USERNAME}" ]] && die "Se necesita el nombre de usuario."

read -rp "Nombre del repositorio [${REPO_NAME}]: " REPO
[[ -z "${REPO}" ]] && REPO="${REPO_NAME}"

read -rp "Visibilidad (public/private) [private]: " VISIBILITY
[[ -z "${VISIBILITY}" ]] && VISIBILITY="private"

# ---- 3. Crear el repositorio remoto ---------------------------------------
if command -v gh >/dev/null 2>&1; then
    ok "GitHub CLI disponible; creando repositorio '${REPO}' (${VISIBILITY})..."
    gh repo create "${REPO}" --"${VISIBILITY}" --source=. --remote=origin --push || \
        die "gh repo create falló. Verifique el login: gh auth login"
else
    ok "gh no está instalado. Crear el repositorio manualmente y luego:"
    echo "    git remote add origin https://github.com/${USERNAME}/${REPO}.git"
    echo "    git push -u origin main"
    echo "  (o configurar SSH: git@github.com:${USERNAME}/${REPO}.git)"
    # Si el usuario ya lo creó, igual intentamos el remote y el push.
    if [[ -z "$(git remote get-url origin 2>/dev/null || true)" ]]; then
        git remote add origin "https://github.com/${USERNAME}/${REPO}.git" 2>/dev/null || true
    fi
fi

# ---- 4. Push con tag (reglas de LEARNINGS.md) ------------------------------
if [[ -n "$(git remote get-url origin 2>/dev/null || true)" ]]; then
    ok "Haciendo push de main y del tag v${VERSION}..."
    git push -u origin main || die "Push falló. Revise las credenciales (git config credential.helper)."
    if ! git rev-parse "v${VERSION}" >/dev/null 2>&1; then
        git tag "v${VERSION}"
    fi
    git push origin "v${VERSION}" || true
    ok "Listo: https://github.com/${USERNAME}/${REPO}"
else
    ok "No se configuró remote; el repo queda local. Cree el repo y añada el remote."
fi
