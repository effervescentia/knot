var $knot = require("@knot/runtime");
var emptyStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {};
                  })();
exports.emptyStyle = emptyStyle;
var colorStyle = (function () {
                    var $ = $knot.style.styleExpressionPlugin;
                    var $rules$ = $knot.style.styleRulePlugin;
                    return {
                      mockColor: $rules$.mockColor($.mockRed)
                    };
                  })();
exports.colorStyle = colorStyle;
var rawColorStyle = (function () {
                       var $ = $knot.style.styleExpressionPlugin;
                       var $rules$ = $knot.style.styleRulePlugin;
                       return {
                         mockColor: $rules$.mockColor("#dac33d")
                       };
                     })();
exports.rawColorStyle = rawColorStyle;
