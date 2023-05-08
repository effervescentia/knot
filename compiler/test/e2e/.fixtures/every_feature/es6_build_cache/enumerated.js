import $knot from "@knot/runtime";
var CardSuites = {
  Spades: function Spades() {
            return [CardSuites.Spades];
          },
  Clubs: function Clubs() {
           return [CardSuites.Clubs];
         },
  Diamonds: function Diamonds() {
              return [CardSuites.Diamonds];
            },
  Hearts: function Hearts() {
            return [CardSuites.Hearts];
          }
};
export { CardSuites };
var Account = {
  Verified: function Verified(a, b) {
              return [Account.Verified, a, b];
            },
  Unverified: function Unverified(a) {
                return [Account.Unverified, a];
              }
};
export { Account };
