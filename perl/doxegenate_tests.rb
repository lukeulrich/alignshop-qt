require ".\\doxegenate.rb"
require "test/unit"
 
class DeoxegenateTest < Test::Unit::TestCase
	def test_param_comments_with_default_value
		comment_test(
			'explicit SequenceTextView(QWidget *parent = 0);',
			'/// @param parent QWidget * (Defaults to 0.)')
	end
	
	def test_single_param
		comment_test(
			'void setText(const QString &value);',
			'/// @param value const QString &')
	end
	
	def test_multi_params
		comment_test(
			'PrimerDesignWizard(QWidget *parent, DnaSequence *sequence, int index, const PrimerDesignInput *params);',
			"/// @param parent QWidget *\n/// @param sequence DnaSequence *\n/// @param index int\n/// @param params const PrimerDesignInput *")
	end
	
	def test_multi_line
		comment_test(
			'PrimerDesignWizard(Foo *bar,',
			nil)
	end
	
	def test_return_comment
		comments = LineParser.methodComments('static virtual inline int foo()')
		assert_equal('/// @return int', comments)
	end
	
	def test_comments_ignored
		comments = LineParser.methodComments('//static inline int foo()')
		assert_equal(nil, comments)
	end
		
	def test_explicit_constructors_handled
		comments = LineParser.methodComments(' explicit foo()')
		assert_equal(nil, comments)
	end
	
	def comment_test(line, comment)
		assert_equal(comment, LineParser.getParamComments(line))
	end
end