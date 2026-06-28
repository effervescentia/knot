use super::{
    code::ToCode, CodeFrame, Display, ErrorCode, ErrorContext, ErrorDisplay, ErrorDisplayBuilder,
};
use crate::report::{example, Focus};
use kore::{
    color::{ColoredString, Colorize, Highlight},
    format::SeparateEach,
    invariant,
};
use lang::{ast, types};

impl ToCode for analyze::Error {
    fn to_code(&self) -> ErrorCode {
        match self {
            Self::NotInferrable(..) => ErrorCode::NOT_INFERRABLE,
            Self::NotFound(..) => ErrorCode::NOT_FOUND,
            Self::VariantNotFound(..) => ErrorCode::VARIANT_NOT_FOUND,
            Self::DeclarationNotFound(..) => ErrorCode::DECLARATION_NOT_FOUND,
            Self::NotIndexable(..) => ErrorCode::NOT_INDEXABLE,
            Self::PropertyNotFound(..) => ErrorCode::PROPERTY_NOT_FOUND,
            Self::DuplicateProperty(..) => ErrorCode::DUPLICATE_PROPERTY,
            Self::NotSpreadable(..) => ErrorCode::NOT_SPREADABLE,
            Self::UntypedParameter(..) => ErrorCode::UNTYPED_PARAMETER,
            Self::DefaultValueRejected(..) => ErrorCode::DEFAULT_VALUE_REJECTED,
            Self::NotCallable(..) => ErrorCode::NOT_CALLABLE,
            Self::UnexpectedArgument(..) => ErrorCode::UNEXPECTED_ARGUMENT,
            Self::MissingArgument(..) => ErrorCode::MISSING_ARGUMENT,
            Self::ArgumentRejected(..) => ErrorCode::ARGUMENT_REJECTED,
            Self::NotRenderable(..) => ErrorCode::NOT_RENDERABLE,
            Self::InvalidComponent(..) => ErrorCode::INVALID_COMPONENT,
            Self::ComponentTypo(..) => ErrorCode::COMPONENT_TYPO,
            Self::InvalidAttributes(..) => ErrorCode::INVALID_ATTRIBUTES,
            Self::UnexpectedAttribute(..) => ErrorCode::UNEXPECTED_ATTRIBUTE,
            Self::MissingAttribute(..) => ErrorCode::MISSING_ATTRIBUTE,
            Self::AttributeRejected(..) => ErrorCode::ATTRIBUTE_REJECTED,
            Self::BinaryOperationNotSupported(..) => ErrorCode::BINARY_OPERATION_NOT_SUPPORTED,
            Self::UnaryOperationNotSupported(..) => ErrorCode::UNARY_OPERATION_NOT_SUPPORTED,
            Self::UnexpectedKind(..) => ErrorCode::UNEXPECTED_KIND,
            Self::StyleRuleNotFound(..) => ErrorCode::STYLE_RULE_NOT_FOUND,
            Self::StyleRuleRejected(..) => ErrorCode::STYLE_RULE_REJECTED,
        }
    }
}

impl<'a> Display<'a> for analyze::Error {
    type Context = (&'a lang::CanonicalId, &'a ErrorContext);

    fn display(
        &'a self,
        (
            id,
            ErrorContext {
                root_dir,
                modules,
                nodes,
            },
        ): Self::Context,
    ) -> ErrorDisplay<'a> {
        fn format_list<T, U>(
            items: U,
        ) -> SeparateEach<ColoredString, ColoredString, Vec<ColoredString>>
        where
            T: AsRef<str>,
            U: IntoIterator<Item = T>,
        {
            let success_items = items
                .into_iter()
                .map(|x| x.as_ref().success())
                .collect::<Vec<_>>();

            SeparateEach(
                ", ".subtle(),
                if success_items.is_empty() {
                    vec!["(None)".subtle()]
                } else {
                    success_items
                },
            )
        }

        let code_frame = |focus, id| {
            let range = *nodes
                .get(id)
                .unwrap_or_else(|| invariant!("node {id:?} does not exist in error context"));
            let (path, text) = modules
                .get(&id.0)
                .unwrap_or_else(|| invariant!("module {:?} does not exist in error context", id.0));

            CodeFrame {
                root_dir,
                path,
                source: text,
                focus,
                range,
                padding: 0,
                color: true,
            }
        };

        let error = ErrorDisplayBuilder::default()
            .code(self.to_code())
            .code_frame(code_frame(Focus::Error, id));

        match self {
            Self::NotInferrable(references) => {
                let mut builder = error.title("Not Inferrable").description(
                    "The type of this expression could not be inferred from other types.",
                );

                for reference_id in references {
                    builder = builder.reference(
                        format!(
                            "This {} is an unresolved dependency.",
                            "expression".highlight()
                        ),
                        code_frame(Focus::Highlight, reference_id),
                    );
                }

                builder
            }

            Self::NotFound(name) => error
                .title("Not Found")
                .description(format!(
                    "This expression references a variable named {} which does not exist.",
                    name.error()
                ))
                .suggestion("Check the spelling of the variable name or declare a new variable.")
                .example(example::local_variables(name)),

            Self::VariantNotFound(enum_id, valid_variants, expected_variant) => error
                .title("Variant Not Found")
                .description(format!(
                    "There is no variant named {name} declared on this enumerated type.",
                    name = expected_variant.error()
                ))
                .reference(
                    format!("The {} is referenced here.", "enumerated type".highlight()),
                    code_frame(Focus::Highlight, enum_id),
                )
                .suggestion(format!(
                    "Declare a new variant or select an existing variant.

  {} {}",
                    "variants:".subtle(),
                    format_list(valid_variants)
                ))
                .example(example::enumerated_type_variants("First", expected_variant)),

            Self::DeclarationNotFound(module_id, valid_entities, expected_entity) => error
                .title("Declaration Not Found")
                .description(format!(
                    "There are no entities named {name} declared in this module.",
                    name = expected_entity.error()
                ))
                .reference(
                    format!("The {} is referenced here.", "module".highlight()),
                    code_frame(Focus::Highlight, module_id),
                )
                .suggestion(format!(
                    "Declare a new entity or select an existing entity.

  {} {}",
                    "entities:".subtle(),
                    format_list(valid_entities)
                ))
                .example(example::constants(expected_entity)),

            Self::NotIndexable(value_id, name) => error
                .title("Not Indexable")
                .description(format!(
                    "The property {} cannot be accessed. This value does not support properties.",
                    name.error()
                ))
                .reference(
                    format!("The {} is referenced here.", "value".highlight()),
                    code_frame(Focus::Highlight, value_id),
                )
                .suggestion("Remove the property access and use the value directly."),

            // TODO: cannot trigger this error
            Self::PropertyNotFound(value_id, valid_properties, expected_property) => error
                .title("Property Not Found")
                .description(format!(
                    "There is no property named {} on this value.",
                    expected_property.error()
                ))
                .reference(
                    format!("The {} is referenced here.", "value".highlight()),
                    code_frame(Focus::Highlight, value_id),
                )
                .suggestion(format!(
                    "Declare a new property or select an existing property instead.

  {} {}",
                    "properties:".subtle(),
                    format_list(valid_properties)
                ))
                .example(example::object_properties("first", expected_property)),

            Self::DuplicateProperty(name) => error
                .title("Duplicate Property")
                .description(format!(
                    "This expression includes multiple properties named {name}.",
                    name = name.error(),
                ))
                .suggestion(format!(
                    "Remove or rename any repeated {name} properties to avoid conflict.",
                    name = name.highlight()
                ))
                .example(example::object_properties(
                    format!("{}_0", name),
                    format!("{}_1", name),
                )),

            Self::NotSpreadable(node_id) => error
                .title("Not Spreadable")
                .description(format!(
                    "{} cannot be spread because it is not object-like.",
                    "This expression".error()
                ))
                .code_frame(code_frame(Focus::Error, node_id)),

            Self::UntypedParameter(name) => error
                .title("Untyped Parameter")
                .description(format!(
                    "The parameter {} is missing a type annotation.",
                    name.error()
                ))
                .suggestion(format!(
                    "Add an annotation describing the type of the {} parameter.",
                    name.highlight()
                ))
                .example(example::parameter_types()),

            Self::DefaultValueRejected((typedef_id, typedef_type), (default_id, default_type)) => {
                let mut suggestion = format!(
                    "Remove the default value or replace it with a value accepted by {}.",
                    typedef_type.to_string().highlight()
                );

                let example_value = match typedef_type {
                    types::Shape(types::Type::Nil) => Some("nil"),
                    types::Shape(types::Type::Boolean) => Some("true"),
                    types::Shape(types::Type::Integer) => Some("123"),
                    types::Shape(types::Type::Float) => Some("4.56"),
                    types::Shape(types::Type::String) => Some("\"text\""),
                    types::Shape(types::Type::Style) => Some("style {}"),
                    _ => None,
                };

                if let Some(example_value) = example_value {
                    suggestion.push_str(&format!(
                        "\n\n  {} {}",
                        "example:".subtle(),
                        example_value.highlight()
                    ));
                }

                error
                    .title("Default Value Rejected")
                    .description(format!(
                        "The type of this {} does not match the parameter's type.
                
  {} {}
  {} {}",
                        "default value".error(),
                        "expected:".subtle(),
                        typedef_type.to_string().success(),
                        "found:".subtle(),
                        default_type.to_string().error()
                    ))
                    .code_frame(code_frame(Focus::Error, default_id))
                    .reference(
                        format!("The {} is declared here.", "parameter's type".highlight()),
                        code_frame(Focus::Highlight, typedef_id),
                    )
                    .suggestion(suggestion)
            }

            Self::NotCallable(value_id) => error
                .title("Not Callable")
                .description(format!(
                    "This expression is {} and cannot be called.",
                    "not a function".error()
                ))
                .reference(
                    format!("The {} is referenced here.", "expression".highlight()),
                    code_frame(Focus::Highlight, value_id),
                )
                .suggestion(format!(
                    "Remove the arguments and parentheses {} to use this value directly.",
                    "()".highlight()
                )),

            Self::UnexpectedArgument(argument_id, parameter_count) => error
                .title("Unexpected Argument")
                .description(format!(
                    "This {} was not expected by the function call.",
                    "argument".error()
                ))
                .code_frame(code_frame(Focus::Error, argument_id))
                .reference(
                    format!(
                        "The {} being called here expects {} arguments.",
                        "function".highlight(),
                        parameter_count.to_string().highlight()
                    ),
                    code_frame(Focus::Highlight, id),
                )
                .suggestion("Remove this argument from the function call."),

            Self::MissingArgument(parameter_id, parameter_type) => error
                .title("Missing Argument")
                .description(format!(
                    "This {} expects an argument of type {} that was not provided.",
                    "function call".error(),
                    parameter_type.to_string().success()
                ))
                .reference(
                    "The unfulfilled parameter type is declared here.",
                    code_frame(Focus::Highlight, parameter_id),
                )
                .suggestion(format!(
                    "Add an argument of type {} to the function call.",
                    parameter_type.to_string().highlight()
                )),

            Self::ArgumentRejected(
                (parameter_id, parameter_type),
                (argument_id, argument_type),
            ) => error
                .title("Argument Rejected")
                .description(format!(
                    "The type of {} does not match the expected type.

  {} {}
  {} {}",
                    "this argument".error(),
                    "expected:".subtle(),
                    parameter_type.to_string().success(),
                    "actual:".subtle(),
                    argument_type.to_string().error(),
                ))
                .code_frame(code_frame(Focus::Error, argument_id))
                .reference(
                    format!(
                        "The {} of the argument is declared here.",
                        "expected type".highlight()
                    ),
                    code_frame(Focus::Highlight, parameter_id),
                )
                .suggestion(format!(
                    "Replace this argument with a value of type {}.",
                    parameter_type.to_string().highlight()
                )),

            Self::NotRenderable(node_id) => error
                .title("Not Renderable")
                .description(format!(
                    "The {} of this view cannot be rendered.",
                    "return value".error()
                ))
                .code_frame(code_frame(Focus::Error, node_id))
                .reference(
                    format!("The {} is declared here.", "view".highlight()),
                    code_frame(Focus::Highlight, id),
                )
                .suggestion(format!(
                    "Return a value that can be rendered.
This includes {} and primitive types: {}.",
                    "element".success(),
                    format_list(["nil", "boolean", "integer", "float", "string"]),
                )),

            Self::InvalidComponent(name, actual_type) => {
                error.title("Invalid Component").description(format!(
                    "The variable {} does not reference a valid component type.
                    
  {} {}
  {} {}",
                    name.error(),
                    "expected:".subtle(),
                    "view {}".success(),
                    "actual:".subtle(),
                    actual_type.to_string().error(),
                ))
            }

            Self::ComponentTypo(start_tag, end_tag) => error
                .title("Component Typo")
                .description(format!(
                    "The start {} and end {} tags of this component do not match.",
                    format!("<{start_tag}>").error(),
                    format!("</{end_tag}>").error(),
                ))
                .suggestion("Change the end tag to match the start tag.")
                .example(example::open_component(start_tag)),

            // this is only possible when parsing type modules
            Self::InvalidAttributes(_) => error
                .title("Invalid Attributes")
                .description("The provided attributes are not an object type."),

            Self::UnexpectedAttribute(attribute_id, name) => error
                .title("Unexpected Attribute")
                .description(format!(
                    "An attribute named {} was not expected by this component.",
                    name.error()
                ))
                .code_frame(code_frame(Focus::Error, attribute_id))
                .reference(
                    format!("The {} is rendered here.", "component".highlight()),
                    code_frame(Focus::Highlight, id),
                )
                .suggestion(format!(
                    "Remove the attribute {} from the component.",
                    name.highlight()
                )),

            Self::MissingAttribute(attribute_id, attribute_name, attribute_type) => error
                .title("Missing Attribute")
                .description(format!(
                    "This component expects an attribute {} of type {} that was not provided.",
                    attribute_name.success(),
                    attribute_type.to_string().success()
                ))
                .reference(
                    format!(
                        "The {} of the missing attribute is declare here.",
                        "expected type".highlight()
                    ),
                    code_frame(Focus::Highlight, attribute_id),
                )
                .suggestion(format!(
                    "Add an attribute {} of type {}.",
                    attribute_name.highlight(),
                    attribute_type.to_string().highlight()
                ))
                .example(example::component_attributes("first", attribute_name)),

            Self::AttributeRejected(
                (attribute_id, attribute_name, attribute_type),
                (argument_id, argument_type),
            ) => error
                .title("Attribute Rejected")
                .description(format!(
                    "The type of the attribute {} does not match the expected type.
                
  {} {}
  {} {}",
                    attribute_name.error(),
                    "expected:".subtle(),
                    attribute_type.to_string().success(),
                    "actual:".subtle(),
                    argument_type.to_string().error(),
                ))
                .code_frame(code_frame(Focus::Error, argument_id))
                .reference(
                    format!(
                        "The {} of the attribute is declared here.",
                        "expected type".highlight()
                    ),
                    code_frame(Focus::Highlight, attribute_id),
                )
                .suggestion(format!(
                    "Replace the attribute {} with a value of type {}.",
                    attribute_name.highlight(),
                    attribute_type.to_string().highlight()
                )),

            Self::StyleRuleNotFound(name) => error
                .title("Style Rule Not Found")
                .description(format!(
                    "There is no style rule named {} in the current platform.",
                    name.error(),
                ))
                .suggestion(
                    "Check the documentation for your selected platform to find an appropriate styling rule.",
                ),

            Self::StyleRuleRejected(name, expected_type, actual_type) => error
                .title("Style Rule Rejected")
                .description(format!(
                    "The value of style rule {} does not match the expected type.
                
  {} {} {} {}
  {} {}",
                    name.error(),
                    "expected:".subtle(),
                    expected_type.to_string().success(),
                    "or".subtle(),
                    "string".success(),
                    "actual:".subtle(),
                    actual_type.to_string().error(),
                ))
                .suggestion(format!(
                    "Replace the value of {} with one of type {}.
Alternatively you can use a raw {} value for styling rules though it will bypass type safety.",
                    name.highlight(),
                    expected_type.to_string().highlight(),
                    "string".highlight(),
                )),

            Self::BinaryOperationNotSupported(op, (lhs_id, lhs_type), (rhs_id, rhs_type)) => {
                let mut builder =
                    error
                        .title("Binary Operation Not Supported")
                        .description(format!(
                            "The operator {} cannot be applied to the arguments provided.",
                            op.to_string().highlight().bold()
                        ));

                if let Some(type_) = lhs_type {
                    builder = builder.reference(
                        format!(
                            "The left-hand side of this operation has type {}.",
                            type_.to_string().highlight()
                        ),
                        code_frame(Focus::Highlight, lhs_id),
                    );
                }

                if let Some(type_) = rhs_type {
                    builder = builder.reference(
                        format!(
                            "The right-hand side of this operation has type {}.",
                            type_.to_string().highlight()
                        ),
                        code_frame(Focus::Highlight, rhs_id),
                    );
                }

                builder.suggestion(format!(
                    "Make sure that both arguments have the correct types for this operator.\n{}",
                    match op {
                        ast::BinaryOperator::Add
                        | ast::BinaryOperator::Subtract
                        | ast::BinaryOperator::Multiply
                        | ast::BinaryOperator::Divide
                        | ast::BinaryOperator::Exponent
                        | ast::BinaryOperator::LessThan
                        | ast::BinaryOperator::LessThanOrEqual
                        | ast::BinaryOperator::GreaterThan
                        | ast::BinaryOperator::GreaterThanOrEqual => format!(
                            "The operator {} can only be applied to numbers like {} or {}.",
                            op.to_string().highlight().bold(),
                            "integer".success(),
                            "float".success(),
                        ),

                        ast::BinaryOperator::And | ast::BinaryOperator::Or => format!(
                            "The operator {} can only be applied to {} values.",
                            op.to_string().highlight().bold(),
                            "boolean".success(),
                        ),

                        ast::BinaryOperator::Equal | ast::BinaryOperator::NotEqual => format!(
                            "The operator {} can only be applied to values of the same type.",
                            op.to_string().highlight().bold(),
                        ),
                    }
                ))
            }

            Self::UnaryOperationNotSupported(op, (rhs_id, rhs_type)) => {
                let mut builder =
                    error
                        .title("Unary Operation Not Supported")
                        .description(format!(
                            "The operator {} cannot be applied to the argument provided.",
                            op.to_string().highlight().bold()
                        ));

                if let Some(type_) = rhs_type {
                    builder = builder.reference(
                        format!(
                            "The right-hand side of this operation has type {}.",
                            type_.to_string().highlight()
                        ),
                        code_frame(Focus::Highlight, rhs_id),
                    );
                }

                builder.suggestion(format!(
                    "Replace the argument with one matching the expected type for this operator.
{}",
                    match op {
                        ast::UnaryOperator::Absolute | ast::UnaryOperator::Negate => format!(
                            "The operator {} can only be applied to numbers like {} or {}.",
                            op.to_string().highlight().bold(),
                            "integer".success(),
                            "float".success(),
                        ),

                        ast::UnaryOperator::Not => format!(
                            "The operator {} can only be applied to {} values.",
                            op.to_string().highlight().bold(),
                            "boolean".success(),
                        ),
                    }
                ))
            }

            Self::UnexpectedKind(_, kind) => error.title("Unexpected Kind").description(format!(
                "This expression should be a {} but instead found a {}.",
                format!("{kind} expression").success(),
                format!("{kind} expression", kind = kind.invert()).error()
            )),
        }
        .build()
    }
}
