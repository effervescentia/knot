use super::{
    data::{Action, Data},
    state::State,
};
use crate::ast;
use lang::{types::Type, Fragment, NodeId};

pub fn infer(state: &State, declarations: &[NodeId]) -> Action {
    let typed_declarations = declarations
        .iter()
        .map(|x| match state.fragments.get(x)? {
            (
                _,
                Fragment::Declaration(
                    ast::Declaration::TypeAlias { storage, .. }
                    | ast::Declaration::Enumerated { storage, .. }
                    | ast::Declaration::Constant { storage, .. }
                    | ast::Declaration::Function { storage, .. }
                    | ast::Declaration::View { storage, .. }
                    | ast::Declaration::Module { storage, .. },
                ),
            ) => {
                let (kind, _) = state.types.get(x)?;

                Some((storage.binding.clone(), *kind, *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>();

    typed_declarations
        .map(|xs| Action::Infer(Data::Local(Type::Module(xs))))
        .unwrap_or(Action::Skip)
}
