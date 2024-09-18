import { css } from "@emotion/css";

export const createStyle = {
  "1.0": (styleObject) => styleObject,
};

export const bindStyle = {
  "1.0": (styleObject) => {
    const cssObject = Object.fromEntries(Object.entries(styleObject).map(([key, value]) => {
      const cssValue = typeof value !== "string" && "toCSS" in value ? value.toCSS() : value;

      return [key, cssValue];
    }));


    return css(cssObject);
  },
};
