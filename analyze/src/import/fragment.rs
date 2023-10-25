use crate::{fragment::Fragment, register::ToFragment};
use lang::ast::ImportNodeValue;

impl ToFragment for ImportNodeValue {
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Import(self.clone())
    }
}
