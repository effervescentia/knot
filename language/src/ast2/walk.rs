use super::{raw, Range};

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct NodeId(pub usize);

pub trait Visit {
    type Expr;
    type Stmt;
    type Comp;
    type TExpr;
    type Param;
    type Decl;
    type Imp;
    type Mod;

    fn expression(
        self,
        x: super::Expression<Self::Expr, Self::Stmt, Self::Comp>,
        r: Range,
    ) -> (Self::Expr, Self);

    fn statement(self, x: super::Statement<Self::Expr>, r: Range) -> (Self::Stmt, Self);

    fn component(self, x: super::Component<Self::Expr, Self::Comp>, r: Range)
        -> (Self::Comp, Self);

    fn type_expression(
        self,
        x: super::TypeExpression<Self::TExpr>,
        r: Range,
    ) -> (Self::TExpr, Self);

    fn parameter(
        self,
        x: super::Parameter<Self::Expr, Self::TExpr>,
        r: Range,
    ) -> (Self::Param, Self);

    fn declaration(
        self,
        x: super::Declaration<Self::Expr, Self::Param, Self::Mod, Self::TExpr>,
        r: Range,
    ) -> (Self::Decl, Self);

    fn import(self, x: super::Import, r: Range) -> (Self::Imp, Self);

    fn module(self, x: super::Module<Self::Imp, Self::Decl>, r: Range) -> (Self::Mod, Self);
}

pub trait Walk<Visitor>
where
    Visitor: Visit,
{
    type Output;

    fn walk(self, visitor: Visitor) -> (Self::Output, Visitor);
}

impl<Key, Target, Visitor> Walk<Visitor> for (Key, Target)
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = (Key, Target::Output);

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (key, x) = self;
        let (x, v) = x.walk(v);

        ((key, x), v)
    }
}

impl<Target, Visitor> Walk<Visitor> for Option<Target>
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = Option<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        if let Some(x) = self {
            let (x, v) = x.walk(v);
            (Some(x), v)
        } else {
            (None, v)
        }
    }
}

impl<Target, Visitor> Walk<Visitor> for Vec<Target>
where
    Target: Walk<Visitor>,
    Visitor: Visit,
{
    type Output = Vec<Target::Output>;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        self.into_iter().fold((vec![], v), |(mut acc, v), x| {
            let (x, v) = x.walk(v);
            acc.push(x);
            (acc, v)
        })
    }
}

impl<Visitor> Walk<Visitor> for raw::Expression
where
    Visitor: Visit,
{
    type Output = Visitor::Expr;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Expression::Primitive(x) => v.expression(super::Expression::Primitive(x), range),

            super::Expression::Identifier(x) => {
                v.expression(super::Expression::Identifier(x), range)
            }

            super::Expression::Group(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Group(Box::new(x)), range)
            }

            super::Expression::Closure(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(super::Expression::Closure(xs), range)
            }

            super::Expression::UnaryOperation(op, x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::UnaryOperation(op, Box::new(x)), range)
            }

            super::Expression::BinaryOperation(op, l, r) => {
                let (l, v) = l.walk(v);
                let (r, v) = r.walk(v);

                v.expression(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    range,
                )
            }

            super::Expression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.expression(
                    super::Expression::PropertyAccess(Box::new(x), property),
                    range,
                )
            }

            super::Expression::FunctionCall(x, arguments) => {
                let (x, v) = x.walk(v);
                let (arguments, v) = arguments.walk(v);

                v.expression(
                    super::Expression::FunctionCall(Box::new(x), arguments),
                    range,
                )
            }

            super::Expression::Component(x) => {
                let (x, v) = x.walk(v);

                v.expression(super::Expression::Component(Box::new(x)), range)
            }

            super::Expression::Style(xs) => {
                let (xs, v) = xs.walk(v);

                v.expression(super::Expression::Style(xs), range)
            }
        }
    }
}

impl<Visitor> Walk<Visitor> for raw::Statement
where
    Visitor: Visit,
{
    type Output = Visitor::Stmt;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Statement::Expression(x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Expression(x), range)
            }

            super::Statement::Variable(binding, x) => {
                let (x, v) = x.walk(v);

                v.statement(super::Statement::Variable(binding, x), range)
            }
        }
    }
}

impl<Visitor> Walk<Visitor> for raw::Component
where
    Visitor: Visit,
{
    type Output = Visitor::Comp;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor)
    where
        Visitor: Visit,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::Component::Text(x) => v.component(super::Component::Text(x), range),

            super::Component::Expression(x) => {
                let (x, v) = x.walk(v);

                v.component(super::Component::Expression(x), range)
            }

            super::Component::Fragment(xs) => {
                let (xs, v) = xs.walk(v);

                v.component(super::Component::Fragment(xs), range)
            }

            super::Component::ClosedElement(tag, attributes) => {
                let (attributes, v) = attributes.walk(v);

                v.component(super::Component::ClosedElement(tag, attributes), range)
            }

            super::Component::OpenElement(start_tag, attributes, children, end_tag) => {
                let (attributes, v) = attributes.walk(v);
                let (children, v) = children.walk(v);

                v.component(
                    super::Component::OpenElement(start_tag, attributes, children, end_tag),
                    range,
                )
            }
        }
    }
}

impl<Visitor> Walk<Visitor> for raw::TypeExpression
where
    Visitor: Visit,
{
    type Output = Visitor::TExpr;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor)
    where
        Visitor: Visit,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::TypeExpression::Primitive(x) => {
                v.type_expression(super::TypeExpression::Primitive(x), range)
            }

            super::TypeExpression::Identifier(x) => {
                v.type_expression(super::TypeExpression::Identifier(x), range)
            }

            super::TypeExpression::Group(x) => {
                let (x, v) = x.walk(v);

                v.type_expression(super::TypeExpression::Group(Box::new(x)), range)
            }

            super::TypeExpression::PropertyAccess(x, property) => {
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::PropertyAccess(Box::new(x), property),
                    range,
                )
            }

            super::TypeExpression::Function(parameters, x) => {
                let (parameters, v) = parameters.walk(v);
                let (x, v) = x.walk(v);

                v.type_expression(
                    super::TypeExpression::Function(parameters, Box::new(x)),
                    range,
                )
            }
        }
    }
}

impl<Visitor> Walk<Visitor> for raw::Parameter
where
    Visitor: Visit,
{
    type Output = Visitor::Param;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node {
            value:
                super::Parameter {
                    name,
                    value_type,
                    default_value,
                },
            range,
        } = self.0;
        let (value_type, v) = value_type.walk(v);
        let (default_value, v) = default_value.walk(v);

        v.parameter(
            super::Parameter {
                name,
                value_type,
                default_value,
            },
            range,
        )
    }
}

impl<Visitor> Walk<Visitor> for raw::Declaration
where
    Visitor: Visit,
{
    type Output = Visitor::Decl;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Declaration::TypeAlias { storage, value } => {
                let (value, v) = value.walk(v);

                v.declaration(super::Declaration::TypeAlias { storage, value }, range)
            }

            super::Declaration::Constant {
                storage,
                value_type,
                value,
            } => {
                let (value_type, v) = value_type.walk(v);
                let (value, v) = value.walk(v);

                v.declaration(
                    super::Declaration::Constant {
                        storage,
                        value_type,
                        value,
                    },
                    range,
                )
            }

            super::Declaration::Enumerated { storage, variants } => {
                let (variants, v) = variants.walk(v);

                v.declaration(super::Declaration::Enumerated { storage, variants }, range)
            }

            super::Declaration::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                let (parameters, v) = parameters.walk(v);
                let (body_type, v) = body_type.walk(v);
                let (body, v) = body.walk(v);

                v.declaration(
                    super::Declaration::Function {
                        storage,
                        parameters,
                        body_type,
                        body,
                    },
                    range,
                )
            }

            super::Declaration::View {
                storage,
                parameters,
                body,
            } => {
                let (parameters, v) = parameters.walk(v);
                let (body, v) = body.walk(v);

                v.declaration(
                    super::Declaration::View {
                        storage,
                        parameters,
                        body,
                    },
                    range,
                )
            }

            super::Declaration::Module { storage, value } => {
                let (value, v) = value.walk(v);

                v.declaration(super::Declaration::Module { storage, value }, range)
            }
        }
    }
}

impl<Visitor> Walk<Visitor> for raw::Import
where
    Visitor: Visit,
{
    type Output = Visitor::Imp;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node {
            value:
                super::Import {
                    source,
                    path,
                    alias,
                },
            range,
        } = self.0;

        v.import(
            super::Import {
                source,
                path,
                alias,
            },
            range,
        )
    }
}

impl<Visitor> Walk<Visitor> for raw::Module
where
    Visitor: Visit,
{
    type Output = Visitor::Mod;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let raw::Node {
            value:
                super::Module {
                    imports,
                    declarations,
                },
            range,
        } = self.0;
        let (imports, v) = imports.walk(v);
        let (declarations, v) = declarations.walk(v);

        v.module(
            super::Module {
                imports,
                declarations,
            },
            range,
        )
    }
}
