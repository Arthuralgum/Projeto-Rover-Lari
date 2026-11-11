import React, { useCallback, useEffect, useRef, useState } from 'react';
import {
  Animated,
  Easing,
  Pressable,
  StatusBar,
  StyleSheet,
  View,
  useWindowDimensions,
} from 'react-native';
import { useKeepAwake } from 'expo-keep-awake';

const WHITE = '#FFFFFF';

/* Adicionar novos rostos no futuro caso necessário */
const FACES = ['neutral', 'sleeping', 'happy'];

function ZGlyph({ size, thickness }) {
  const diagonal = (size - thickness) * Math.SQRT2;
  const bar = {
    position: 'absolute',
    backgroundColor: WHITE,
    borderRadius: thickness / 2,
  };
  return (
    <View style={{ width: size, height: size }}>
      <View style={[bar, { top: 0, left: 0, width: size, height: thickness }]} />
      <View style={[bar, { bottom: 0, left: 0, width: size, height: thickness }]} />
      <View
        style={[
          bar,
          {
            top: (size - thickness) / 2,
            left: (size - diagonal) / 2,
            width: diagonal,
            height: thickness,
            transform: [{ rotate: '-45deg' }],
          },
        ]}
      />
    </View>
  );
}

/** Z que sobe e desce devagarinho enquanto o robô dorme. */
function FloatingZ({ boxStyle, size, thickness, phase }) {
  const t = useRef(new Animated.Value(phase)).current;

  useEffect(() => {
    const anim = Animated.loop(
      Animated.sequence([
        Animated.timing(t, {
          toValue: 1,
          duration: 1300,
          easing: Easing.inOut(Easing.sin),
          useNativeDriver: true,
        }),
        Animated.timing(t, {
          toValue: 0,
          duration: 1300,
          easing: Easing.inOut(Easing.sin),
          useNativeDriver: true,
        }),
      ])
    );
    anim.start();
    return () => anim.stop();
  }, [t]);


  const opacity = t.interpolate({ inputRange: [0, 1], outputRange: [0.65, 1] });
  const translateY = t.interpolate({ inputRange: [0, 1], outputRange: [0, -size * 0.35] });

  return (
    <Animated.View style={[boxStyle, { opacity, transform: [{ translateY }] }]}>
      <ZGlyph size={size} thickness={thickness} />
    </Animated.View>
  );
}

export default function App() {
  useKeepAwake(); // o tablet não apaga a tela

  const { width, height } = useWindowDimensions();

  const [face, setFace] = useState('neutral');
  const faceRef = useRef('neutral');
  const switching = useRef(false);
  const lookTimer = useRef(null);

  const fade = useRef(new Animated.Value(1)).current; // troca de rosto
  const blink = useRef(new Animated.Value(1)).current; // piscar (escala vertical dos olhos)
  const lookX = useRef(new Animated.Value(0)).current; // olhar para os lados
  const lookY = useRef(new Animated.Value(0)).current;
  const breath = useRef(new Animated.Value(1)).current; // respiração (dormindo)

  // `u` é o tamanho de uma unidade em pixels.
  const u = (Math.min(width, height) / 100) * 1.15;
  const uRef = useRef(u);
  uRef.current = u;

  // Quando deitarmos o tablet, afasta um pouco os olhos para aproveitar a largura.
  const spread = Math.min(1.4, Math.max(1, width / height));

  // Posiciona uma peça pelo centro (x, y) e tamanho (w, h), tudo em unidades.
  const place = (x, y, w, h) => ({
    position: 'absolute',
    left: width / 2 + (x - 50) * u * spread - (w * u) / 2,
    top: height / 2 + (y - 56) * u - (h * u) / 2,
    width: w * u,
    height: h * u,
  });

  // ---------- trocar de rosto ----------
  const goTo = useCallback(
    (next) => {
      if (!FACES.includes(next) || switching.current || next === faceRef.current) return;
      switching.current = true;
      Animated.timing(fade, { toValue: 0, duration: 130, useNativeDriver: true }).start(() => {
        faceRef.current = next;
        setFace(next);
        Animated.timing(fade, { toValue: 1, duration: 200, useNativeDriver: true }).start(() => {
          switching.current = false;
        });
      });
    },
    [fade]
  );

  const nextFace = useCallback(() => {
    const i = FACES.indexOf(faceRef.current);
    goTo(FACES[(i + 1) % FACES.length]);
  }, [goTo]);

  // ---------- olhar para um ponto e voltar ----------
  const lookAt = useCallback(
    (x, y) => {
      clearTimeout(lookTimer.current);
      const spring = (value, toValue) =>
        Animated.spring(value, { toValue, speed: 14, bounciness: 4, useNativeDriver: true });
      spring(lookX, x).start();
      spring(lookY, y).start();
      lookTimer.current = setTimeout(() => {
        spring(lookX, 0).start();
        spring(lookY, 0).start();
      }, 1100);
    },
    [lookX, lookY]
  );

  useEffect(() => () => clearTimeout(lookTimer.current), []);

  // ---------- comportamentos de cada rosto ----------
  useEffect(() => {
    if (face === 'sleeping') {
      lookX.setValue(0);
      lookY.setValue(0);
      blink.setValue(1);
      const anim = Animated.loop(
        Animated.sequence([
          Animated.timing(breath, {
            toValue: 1.035,
            duration: 2000,
            easing: Easing.inOut(Easing.sin),
            useNativeDriver: true,
          }),
          Animated.timing(breath, {
            toValue: 1,
            duration: 2000,
            easing: Easing.inOut(Easing.sin),
            useNativeDriver: true,
          }),
        ])
      );
      anim.start();
      return () => {
        anim.stop();
        breath.setValue(1);
      };
    }

    // neutro e feliz: piscar e dar umas olhadinhas aleatórias
    let blinkTimer;
    let glanceTimer;

    const scheduleBlink = () => {
      blinkTimer = setTimeout(() => {
        Animated.sequence([
          Animated.timing(blink, { toValue: 0.08, duration: 80, useNativeDriver: true }),
          Animated.timing(blink, { toValue: 1, duration: 120, useNativeDriver: true }),
        ]).start();
        scheduleBlink();
      }, 2200 + Math.random() * 3800);
    };

    const scheduleGlance = () => {
      glanceTimer = setTimeout(() => {
        const k = uRef.current;
        lookAt((Math.random() * 2 - 1) * 4 * k, (Math.random() * 2 - 1) * 2 * k);
        scheduleGlance();
      }, 3500 + Math.random() * 4500);
    };

    scheduleBlink();
    scheduleGlance();
    return () => {
      clearTimeout(blinkTimer);
      clearTimeout(glanceTimer);
    };
  }, [face, blink, breath, lookX, lookY, lookAt]);

  // ---------- toque na tela: olha pro toque e troca de rosto ----------
  const onPress = (e) => {
    const { pageX, pageY } = e.nativeEvent;
    const nx = (pageX - width / 2) / (width / 2);
    const ny = (pageY - height / 2) / (height / 2);
    lookAt(nx * 4 * uRef.current, ny * 2.5 * uRef.current);
    nextFace();
  };

  // ---------- peças ----------
  const eyeStyle = (x) => [
    place(x, 44, 17, 24),
    { backgroundColor: WHITE, borderRadius: 8 * u },
    { transform: [{ translateX: lookX }, { translateY: lookY }, { scaleY: blink }] },
  ];

  const solid = { backgroundColor: WHITE };

  // coradinho
  const cheek = (x, y, slant) => {
    const stroke = [
      solid,
      { borderRadius: u, transform: [{ rotate: slant === '/' ? '-55deg' : '55deg' }] },
    ];
    return (
      <>
        <View style={[place(x - 2.6, y + 0.8, 7, 1.9), ...stroke]} />
        <View style={[place(x + 2.6, y + 0.8, 7, 1.9), ...stroke]} />
      </>
    );
  };

  const renderFace = () => {
    if (face === 'sleeping') {
      return (
        <>
          <FloatingZ boxStyle={place(12, 28, 7, 7)} size={7 * u} thickness={1.7 * u} phase={0} />
          <FloatingZ boxStyle={place(26, 15, 11, 11)} size={11 * u} thickness={2.2 * u} phase={0.5} />

          {/* olhos fechados */}
          <View
            style={[
              place(25, 50, 16, 3),
              solid,
              { borderRadius: 1.5 * u, transform: [{ rotate: '-8deg' }] },
            ]}
          />
          <View
            style={[
              place(75, 50, 16, 3),
              solid,
              { borderRadius: 1.5 * u, transform: [{ rotate: '8deg' }] },
            ]}
          />

          {/* boquinha em "¬" */}
          <View style={[place(52, 78, 18, 3.2), solid, { borderRadius: 1.6 * u }]} />
          <View style={[place(59.5, 81.4, 3.2, 10), solid, { borderRadius: 1.6 * u }]} />
        </>
      );
    }

    if (face === 'happy') {
      return (
        <>
          <Animated.View style={eyeStyle(27)} />
          <Animated.View style={eyeStyle(73)} />
          {cheek(19, 68, '\\')}
          {cheek(81, 68, '/')}
          <View
            style={[
              place(50, 76, 15, 3.6),
              solid,
              {
                borderTopLeftRadius: 1 * u,
                borderTopRightRadius: 1 * u,
                borderBottomLeftRadius: 3 * u,
                borderBottomRightRadius: 3 * u,
              },
            ]}
          />
        </>
      );
    }

    // neutral
    return (
      <>
        <Animated.View style={eyeStyle(27)} />
        <Animated.View style={eyeStyle(73)} />
        <View style={[place(50, 77, 4.5, 3.6), solid, { borderRadius: 1.8 * u }]} />
      </>
    );
  };

  return (
    <View style={styles.root}>
      <StatusBar hidden />
      <Pressable style={StyleSheet.absoluteFill} onPress={onPress}>
        <Animated.View
          style={[StyleSheet.absoluteFill, { opacity: fade, transform: [{ scale: breath }] }]}
        >
          {renderFace()}
        </Animated.View>
      </Pressable>
    </View>
  );
}

const styles = StyleSheet.create({
  root: {
    flex: 1,
    backgroundColor: '#000000',
  },
});

/* Caso esteja vendo isso, ainda não foi criado nenhum sistema de conexão com o esp32 nesse app, apenas os rostos, no futuro é necessário criar um http para passar um Json para o esp ou semelhante para travar os motores para as pessoas conseguirem ver as informações no tablet*/