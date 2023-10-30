use super::ScopeContext;
use crate::register::Identify;
use lang::ast::ImportNodeValue;

impl Identify<Self> for ImportNodeValue {
    fn identify(&self, _ctx: &ScopeContext) -> Self {
        self.clone()
    }
}
