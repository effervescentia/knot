import $knot from "@knot/runtime";
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {};
                  })();
export { emptyStyle };
var redStyle = (function () {
                  var $ = $knot.style.styleExpressionPlugin;
                  var $rules$ = $knot.style.styleRulePlugin;
                  return {
                    mockColor: $rules$.mockColor($.mockRed)
                  };
                })();
export { redStyle };
var yellowStyle = (function () {
                     var $ = $knot.style.styleExpressionPlugin;
                     var $rules$ = $knot.style.styleRulePlugin;
                     return {
                       mockBackgroundColor: $rules$.mockBackgroundColor(
                       "#dac33d")
                     };
                   })();
export { yellowStyle };
function Component($props$) {
  return null;
};
export { Component };
var EmptyComponent = $knot.style.bindStyle(Component, emptyStyle);
export { EmptyComponent };
var RedAndYellowComponent = $knot.style.bindStyle($knot.style.bindStyle(
                                                  Component,
                                                  redStyle),
                                                  yellowStyle);
export { RedAndYellowComponent };
var StyleLiteralComponent = $knot.style.bindStyle(Component,
                                                  (function () {
                                                     var $ = $knot.style.styleExpressionPlugin;
                                                     var $rules$ = $knot.style.styleRulePlugin;
                                                     return {
                                                       mockColor: $rules$.mockColor(
                                                       "#adec22")
                                                     };
                                                   })());
export { StyleLiteralComponent };
