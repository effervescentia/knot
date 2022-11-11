var $knot = require("@knot/runtime");
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {};
                  })();
exports.emptyStyle = emptyStyle;
var redStyle = (function () {
                  var $ = $knot.style.styleExpressionPlugin;
                  var $rules$ = $knot.style.styleRulePlugin;
                  return {
                    mockColor: $rules$.mockColor($.mockRed)
                  };
                })();
exports.redStyle = redStyle;
var yellowStyle = (function () {
                     var $ = $knot.style.styleExpressionPlugin;
                     var $rules$ = $knot.style.styleRulePlugin;
                     return {
                       mockBackgroundColor: $rules$.mockBackgroundColor(
                       "#dac33d")
                     };
                   })();
exports.yellowStyle = yellowStyle;
function Component($props$) {
  return null;
};
exports.Component = Component;
var EmptyComponent = $knot.style.bindStyle(Component, emptyStyle);
exports.EmptyComponent = EmptyComponent;
var RedAndYellowComponent = $knot.style.bindStyle($knot.style.bindStyle(
                                                  Component,
                                                  redStyle),
                                                  yellowStyle);
exports.RedAndYellowComponent = RedAndYellowComponent;
