# build wut
FROM wiiuenv/devkitppc:20221228 AS final

ENV DEBIAN_FRONTEND=noninteractive \
 PATH=$DEVKITPPC/bin:$PATH \
 WUT_ROOT=$DEVKITPRO/wut
WORKDIR /
COPY --from=wiiuenv/libmocha:20220919112600f3c45c /artifacts $DEVKITPRO
WORKDIR /project
