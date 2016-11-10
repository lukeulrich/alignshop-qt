$alignShopProFile = 'C:\\dev\\qt\\AlignShop\\AlignShop.pro'
$alignShopDirectory = File.dirname($alignShopProFile)

class ProParser
	def printH()
		hs = []
		pro = File.new($alignShopProFile, 'r')
		hs << getFirstHeader(pro)
		
		while (hfile = hFilePath(pro.gets))
			hs << hfile
		end
		hs
	ensure
		pro.close()
	end
	
	def getFirstHeader(pro)
		while (line = pro.gets)
			if (line.start_with?('HEADERS'))
				return hFilePath(line.split('+=')[1])
			end
		end
	end
	
	def hFilePath(line)
		line = line.strip
		if (line.end_with?('\\'))
			line = line.chop.rstrip
		end
		return line.end_with?('.h') ? File.absolute_path(line, $alignShopDirectory) : nil
	end
end

class LineParser
	def LineParser.leadingSpace(line)
		line.split(/\S/, 2).first
	end
	
	def LineParser.methodComments(line)
		if (line.lstrip.start_with?('/'))
			return nil
		end
	
		line = line.sub(' explicit ', ' ')
		paramComment = getParamComments(line)
		returnComment = getReturnComment(line)
		
		if (paramComment && returnComment)
			return paramComment + "\n" + returnComment
		elsif (paramComment)
			return paramComment
		elsif (returnComment)
			return returnComment
		else
			return nil
		end
	end
	
	def LineParser.getReturnComment(line)
		splt = line.partition('(').first.split
		if (splt.last.start_with?('*'))
			splt[-1] = '*'
		elsif (splt.last.start_with?('&'))
			splt[-1] = '&'
		else
			splt.slice!(-1)
		end
		
		if (splt.size > 0)
			return "#{leadingSpace(line)}/// @return #{(splt * ' ')}".sub(' static ', ' ').sub(' inline ', ' ').sub(' virtual ', ' ')
		end
		
		nil
	end
	
	def LineParser.getParamComments(line)
		startIndex = line.index('(')
		endIndex = line.rindex(')')
		
		if (startIndex && endIndex)
			params = line.slice(startIndex + 1, endIndex - startIndex - 1).strip
			if (!params.empty?)
				comment = "#{leadingSpace(line)}/// @param "
				return params.split(',').collect { |v| comment + paramComment(v) } * "\n"
			end
		end
		
		nil
	end

	def LineParser.paramComment(s)
		commentParts = []
		values = s.strip.partition('=')
		pieces = values.first.strip.rpartition(/\s/)
		if (pieces.last.start_with?('*') ||
			pieces.last.start_with?('&'))
			pieces[0] += ' ' + pieces.last[0]
			pieces[-1] = pieces.last[1..-1]
		end
		
		commentParts << pieces.last << pieces.first
		
		if (!values.last.empty?)
			commentParts << "(Defaults to #{values.last.strip}.)"
		end
		
		return commentParts * ' '
	end
end

class LineProcessor
	@inClass
	@classStack

	def initialize
		@inClass = false
		@classStack = []
	end
	
	def process(line)
		if (isReplaceableComment?(line))
			return nil
		end
		
		@inClass = @inClass || isClassDeclaration?(line)
		
		push(line)
		return processMethod(line)
	end
	
	def processMethod(line)
		if (@classStack.size == 1 && isMethod?(line) && generatedComments = LineParser.methodComments(line))
			return "#{generatedComments}\n#{line}"
		else
			return line
		end
	end
		
	def isMethod?(line)
		return line.index('(')
	end
	
	def isReplaceableComment?(line)
		return line.lstrip.start_with?('/// @return', '/// @param')
	end
	
	def push(line)
		if (@inClass)
			wasDeepInClass = @classStack.size > 0
			line = line.lstrip
			if (line.start_with?('{'))
				@classStack.push('{')
			elsif (line.start_with?('}'))
				@classStack.pop()
			end
			
			if (wasDeepInClass)
				@inClass = @classStack.size > 0
			end
		end
	end
	
	def isClassDeclaration?(line)
		return line.lstrip.start_with?('class ') && !line.rstrip.end_with?(';')
	end
end

class HCommenter
	@input
	@output
	
	def comment(filePath)
		printFileName(filePath)
		initializeFile(filePath)
		processFile
	ensure
		closeFile
	end
	
	def printFileName(name)
		puts name
	end
	
	def initializeFile(path)
		tmp = path + '.old'
		if File.exists?(tmp) 
			File.delete(tmp) 
		end
		
		File.rename(path, tmp)
		
		@input = File.new(tmp, 'r')
		@output = File.new(path, 'w+')
	end
	
	def processFile
		processor = LineProcessor.new
		while (line = @input.gets)
			line = processor.process(line)
			if (line)
				@output.puts line
			end
		end
	end
	
	def closeFile
		if (@input) then @input.close() end
		if (@output) then @output.close() end
	end
end

#hfiles = ProParser.new().printH()
#hfiles.each { |h| commenter.comment h }

if (ARGV.first == 'do')
	commenter = HCommenter.new
	Dir.glob('C:/dev/qt/AlignShop/PrimerDesign/*.h').each { |h| commenter.comment h }
	Dir.glob('C:/dev/qt/AlignShop/forms/PrimerDesign/*.h').each { |h| commenter.comment h }
	commenter.comment 'C:/dev/qt/AlignShop/widgets/SequenceTextView.h'
end