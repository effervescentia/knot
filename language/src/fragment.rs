use crate::{ast, NodeId, ScopeId};
use std::collections::BTreeMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    Expression(ast::Expression<NodeId, NodeId, NodeId>),
    Statement(ast::Statement<NodeId>),
    Component(ast::Component<NodeId, NodeId>),
    Parameter(ast::Parameter<String, NodeId, NodeId>),
    Declaration(ast::Declaration<String, NodeId, NodeId, NodeId, NodeId>),
    TypeExpression(ast::TypeExpression<NodeId>),
    Import(ast::Import),
    Module(ast::Module<NodeId, NodeId>),
}

impl Fragment {
    pub fn to_binding(&self) -> Result<Vec<String>, ()> {
        match self {
            Self::Statement(ast::Statement::Variable(binding, ..))
            | Self::Parameter(ast::Parameter { binding, .. }) => Ok(vec![binding.clone()]),

            Self::Declaration(x) => Ok(vec![x.binding().clone()]),

            Self::Import(ast::Import {
                path, alias: None, ..
            }) => Ok(vec![path.last().ok_or(())?.clone()]),

            Self::Import(ast::Import {
                alias: Some(alias), ..
            }) => Ok(vec![alias.clone()]),

            _ => Ok(vec![]),
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct FragmentMap(pub BTreeMap<NodeId, (ScopeId, Fragment)>);

impl FromIterator<(NodeId, (ScopeId, Fragment))> for FragmentMap {
    fn from_iter<T: IntoIterator<Item = (NodeId, (ScopeId, Fragment))>>(iter: T) -> Self {
        Self(BTreeMap::from_iter(iter))
    }
}
