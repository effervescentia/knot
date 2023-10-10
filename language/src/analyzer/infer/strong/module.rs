use crate::{
    analyzer::{context::StrongContext, fragment::Fragment, types::Type},
    ast::Declaration,
};

use super::Strong;

pub fn infer(declarations: &Vec<usize>, ctx: &StrongContext) -> Option<Strong> {
    let typed_declarations = declarations
        .iter()
        .map(|x| match ctx.fragments.0.get(x)? {
            (
                _,
                Fragment::Declaration(
                    Declaration::TypeAlias { name, .. }
                    | Declaration::Enumerated { name, .. }
                    | Declaration::Constant { name, .. }
                    | Declaration::Function { name, .. }
                    | Declaration::View { name, .. }
                    | Declaration::Module { name, .. },
                ),
            ) => {
                let (kind, _) = ctx.refs.get(x)?;

                Some((name.1.clone(), kind.clone(), *x))
            }

            _ => None,
        })
        .collect::<Option<Vec<_>>>()?;

    Some(Ok(Type::Module(typed_declarations)))
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            context::StrongContext, fragment::Fragment, infer::strong::Strong, types::Type, RefKind,
        },
        ast::{
            storage::{Storage, Visibility},
            Declaration,
        },
        test::fixture as f,
    };

    fn infer(declarations: Vec<usize>, ctx: &StrongContext) -> Option<Strong> {
        super::infer(&declarations, ctx)
    }

    #[test]
    fn none_result() {
        let ctx = f::strong_ctx_from(vec![], vec![], vec![]);

        assert_eq!(infer(vec![0], &ctx), None);
    }

    #[test]
    fn declarations() {
        let ctx = f::strong_ctx_from(
            vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("foo")),
                            value_type: None,
                            value: 0,
                        }),
                    ),
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Declaration(Declaration::TypeAlias {
                            name: Storage(Visibility::Public, String::from("bar")),
                            value: 2,
                        }),
                    ),
                ),
            ],
            vec![
                (0, (RefKind::Value, Ok(Type::Boolean))),
                (1, (RefKind::Type, Ok(Type::Integer))),
            ],
            vec![],
        );

        assert_eq!(
            infer(vec![0, 1], &ctx),
            Some(Ok(Type::Module(vec![
                (String::from("foo"), RefKind::Value, 0),
                (String::from("bar"), RefKind::Type, 1)
            ])))
        );
    }
}
