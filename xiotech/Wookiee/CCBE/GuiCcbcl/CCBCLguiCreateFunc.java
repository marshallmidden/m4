/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiCreateFunc.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Creates an array of CCBCLguiFunctions from the ccbcl.pl
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import java.util.Vector;
import java.io.*;

/*===========================================================================
** CCBCLguiCreateFunc
**===========================================================================*/
public class CCBCLguiCreateFunc
{
    /*===========================================================================
    ** CreateFunc       opens the ccbcl.pl and creates functions from it
    **
    ** Inputs:          loc:   - location of ccbcl.pl
    **
    ** Returns:         CCBCLguiFunction[] - array of CCBCLguiFunctions
    **
    ** Throws:          Exception
    **===========================================================================*/
    public static CCBCLguiFunction[] CreateFunc(String loc) throws Exception
    {
        FileInputStream     result;
        int                 sizeOfInput;
        String              returnString;
        byte[]              buffr;
        Vector              store;
        int                 i;
        String              ccbcl = new String("\\ccbcl.pl");
        CCBCLguiFunction[]  returnFuncs = null;
        
        try{
            returnFuncs = new CCBCLguiFunction[1];
            
            /*
            ** if the location passed in is null then we will
            ** use the default
            */
            if (loc == null)
            {
                loc = new String("c:\\projects\\bigfoot\\ccbe");
            }
            ccbcl = loc + ccbcl;
                    
            /*
            ** open the ccbcl.pl file and store it in returnString
            */
            result = new FileInputStream(ccbcl);
            sizeOfInput = result.available();
            
            /*
            ** create the buffer to read the file and the String to store it in
            */
            returnString = new String();
            buffr = new byte[sizeOfInput];
            
            while((result.read(buffr)) != -1)
            {
                returnString += new String(buffr);
            }
            result.close();
            
            /*
            ** call parseFile to parse the ccbcl.pl file and return
            ** a Vector of functions
            */
            store = parseFile(returnString);
            returnFuncs = new CCBCLguiFunction[store.size()];
            
            /*
            ** convert the Vector into an array to return
            */
            for (i = 0; i < store.size(); ++i)
            {
                returnFuncs[i] = (CCBCLguiFunction)store.elementAt(i);
            }
            
        }catch (Exception e)
        {
            e.printStackTrace();
            throw new Exception("ERROR PARSING CCBCL.PL:\n" + e.getMessage());
        }
        
        return returnFuncs;   
    }

    /*===========================================================================
    ** parseFile        parses the ccbcl.pl and creates functions from it
    **
    ** Inputs:          file:   - String of the ccbcl.pl
    **
    ** Returns:         Vector - Vector of CCBCLguiFunctions
    **
    ** Throws:          Exception
    **===========================================================================*/
    private static Vector parseFile(String file) throws Exception
    {
        String              hlpr;
        String              name;
        String              value;
        int                 index;
        char[]              arr             = null;
        int                 i;
        boolean             write;
        boolean             funcNameQuit;
        boolean             funcNameWrite;
        int                 pcount;
        int                 bcount;
        boolean             slash;
        boolean             quit            = false;
        Vector              store           = new Vector();
        Vector              func            = new Vector();
        String              copy            = new String(file);
        
    /*===========================================================================
    ** First Task, Find the functions in the help function and retrieve
    ** the function names as well as the help text for each function
    **===========================================================================*/
        /*
        ** To start we will look for the help function
        ** from here we can get the function names and there
        ** help text
        */
        index = copy.indexOf("sub BuildHelp");
        copy = copy.substring(index);
        
        /*
        ** Our goal now is to parse through these functions
        ** and to create CCBCLguiFunctions in a Vector
        */
        while (!quit)
        {
            /*
            ** initialization
            */
            i               = 0;
            write           = false;
            funcNameQuit    = false;
            funcNameWrite   = false;
            slash           = false;
            hlpr            = new String("");
            name            = new String("");
            pcount          = 0;
            
            /*
            ** we want to move to the "if (" part of this help
            ** text because after this is the function name
            */
            index           = copy.indexOf("$help{");
            if (index == -1)
            {
                break;
            }
            
            copy            = copy.substring(index);
            index           = copy.indexOf("{");
            copy            = copy.substring(index);
            arr             = copy.toCharArray();
            
            
            /*
            ** parse the function name
            */
            while (!funcNameQuit)
            {
                switch (arr[i])
                {
                    case '{':
                    case ' ':
                    case '\"':
                        break;
                    
                    case '}':
                        /*
                        **  if our parenthesis count is == 0 we can stop writing
                        */
                        funcNameQuit = true;
                        break;
                        
                    default:
                        /*
                        **  copy the character into our string
                        */
                        name += arr[i];
                        break;        
                }
                
                ++index;
                ++i;
            }   
            
            for (int myI = 0; myI < 2; ++myI)
            {
                i = 0;
                index           = copy.indexOf("\"");
                if (index == -1)
                {
                    break;
                }
            
                copy            = copy.substring(index);
                arr             = copy.toCharArray();

                /*
                ** We have the function name so now we need to parse its
                ** help text.  We will go until we hit the closing Brace
                */
                while (arr[i] != ';')
                {
                    switch(arr[i])
                    {
                        case '\"':
                            /*
                            **  we start in a state of write = false
                            ** so when we hit our first quotes we want
                            ** to start to write and when we hit again
                            ** we want to stop, and so on, and so on.
                            */
                            write = !write;
                            break;
                        
                        default:
                            /*
                            **  if we are in a write mode lets write!
                            */
                            if(write)
                            {
                                /*
                                ** Our goal in here is to copy the help
                                ** text.  However we would like to convert
                                ** the newlines int the help text to actual
                                ** new lines, so that brings us here to 
                                ** catch the newlines in the event that
                                ** they occur
                                */
                                if (arr[i] == '\\')
                                {
                                    slash = !slash;
                                }
                                else if (arr[i] == 'n')
                                {
                                    if (slash)
                                    {
                                        hlpr += '\n';
                                        slash = !slash;
                                    }
                                    else   
                                    {
                                        hlpr += arr[i];
                                    }
                                }
                                else
                                {
                                    if (slash)
                                    {
                                        slash = !slash;
                                    }
                                     
                                    hlpr += arr[i];
                                }   
                            }
                            break;
                    }
                    ++index;
                    ++i;
                }
                
                copy            = copy.substring(i + 1);
                hlpr += "\n\n";
            }
            
            /*
            ** We need to update our position in the String
            */
            //copy = copy.substring(index);
            
            /*
            ** check to see if we are done yet
            ** in the ccbcl.pl full help text half
            ** the last help text should be within 
            ** 20 characters of the closing function
            ** brace, so we can use this as a stopping
            ** point
            */
            //if ((copy.indexOf("}")) < 20)
            //{
            //    quit = true;
            //}
            
            /*
            ** create a new CCBCLguiFunction and put it in Vector store
            */
            store.add(new CCBCLguiFunction(name,hlpr));
//            System.out.println(name);
//            System.out.println(hlpr);
        }

        
    /*===========================================================================
    ** Second task, now that we have the function names we are going to go
    ** through the functions that are called from the command line directly
    ** and find out what functions in the ccbcl.pl they call.  We will use this
    ** in the third task to determine parameters
    **===========================================================================*/
        /* 
        ** first we want to go to the section of the
        ** program that takes the commands from the
        ** command line
        */
        copy            = new String(file);
        index           = copy.indexOf("CMD_SW:");
        copy            = copy.substring(index);
        quit            = false;
        
        /* parse the function commands and find the functions they call */
        while (!quit)
        {
            /*
            ** initialization
            */
            i               = 0;
            bcount          = 0;
            hlpr            = new String("");
            name            = new String("");
            funcNameQuit    = false;
        
            /*
            ** parse until we get to the
            ** "/^" which is right before
            ** the function name
            */
            index = copy.indexOf("/^");
            copy = copy.substring(index);
            arr             = copy.toCharArray();
            
            /*
            ** jump ahead 2 characters to
            ** surpass "/^"
            */
            index += 2;
            i += 2;
            
            /*
            ** while we don't hit the '$'
            ** character get the function name
            */
            while (arr[i] != '$')
            {
                name += arr[i];
                ++index;
                ++i;
            }
            
            /*
            ** parse until we get to the
            ** "{" which is the body of the
            ** if statement
            */
            index           = copy.indexOf("{");
            copy            = copy.substring(index);
            
            /*
            ** check to see if there may be an
            ** "isActiveConnection()" command
            ** if so we want to jump to the
            ** Brace for this
            */
            try{
                if (copy.indexOf("isActiveConnection()") < 10)
                {
                    index           = copy.indexOf("isActiveConnection()");
                    copy            = copy.substring(index);
                    index           = copy.indexOf("{");
                    copy            = copy.substring(index);
                    arr             = copy.toCharArray();
                }
            }catch (Exception ne)
            {
                /*
                ** if it doesnt' exist we don't care at this point
                */
            }
            
            /*
            ** reset our index
            */
            i = 0;
            
            /* 
            ** get the function that is called
            */
            while (!funcNameQuit)
            {
                switch (arr[i])
                {
                    case '{':
                        /*
                        ** we want to keep a count of
                        ** our braces in the case of
                        ** nested braces
                        */
                        ++bcount;
                        break;
                    
                    case '}':
                        /*
                        ** if we have hit our last closing
                        ** brace set hlpr to "" and quit the
                        ** loop, we did not find a function
                        */
                        if(--bcount <= 0)
                        {
                            hlpr = new String("");
                            funcNameQuit = true;
                        }    
                    
                    case '\n':
                        /*
                        ** if we have hit '\n' or ' '
                        ** set hlpr to "" 
                        */
                        hlpr = new String("");
                        break;
                        
                    case 'f':
                        if ((i > 3) && (arr[i-1] == 'i') 
                            && (arr[i-2] == ' ') && (arr[i-3] == ')'))
                        {
                            i -= 2;
                            //System.out.println ("LOOP\n");
                        }
                        else
                        {
                            hlpr += arr[i];
                            break;
                        }
                        
                    case ';':
                        /*
                        ** if we have hit ';'
                        ** check to see if the last line
                        ** was a function call.  if it is
                        ** parse the function name and exit
                        ** the loop 
                        */
                        if (arr[i-1] == ')')
                        {
                            //System.out.println (hlpr + "\n");
                            arr = hlpr.toCharArray();
                            hlpr = new String("");
                            
                            i = 0;
                            while (arr[i] == ' ')
                            {
                                i++;
                            }
                            while (arr[i] != '(')
                            {
                                hlpr += arr[i++];
                            }
                            funcNameQuit = true;
                        }
                        
                        /*
                        ** else it wasn't a function so
                        ** set hlpr to "" and start again 
                        */
                        else
                        {
                            hlpr = new String("");
                        }
                        break;
                        
                    default:
                        /*
                        ** add the character to hlpr
                        */
                        hlpr += arr[i];
                        break;
                }
                ++i;
                ++index;
            }
            
            /*
            ** We need to update our position in the String
            */
            copy = copy.substring(index);
            
            /*
            ** check to see if we are done yet
            ** in the command portion of the ccbcl.pl
            ** there is a "# default case" at the end
            ** of the commands.  if we are within 40
            ** character of this we are done
            */
            if ((copy.indexOf("# default case")) < 40)
            {
                quit = true;
            }
            
            /* 
            ** create a 2 dimensional string array and put it in Vector func
            */
            //System.out.println("name: " + name + " function: " + hlpr +"\n"); 
            func.add(new String[] {name, hlpr});
        }
        
    /*===========================================================================
    ** Third and final task, we have the function names and their help text
    ** and we also have the function names and the functions they call internally
    ** in the ccbcl.pl.  now we want to compare the two and also go to the 
    ** internal functions and see if we can't determine the parameters that are
    ** used in the internal functions
    **===========================================================================*/
        /*
        ** initialization
        */
        quit = false;
        String[] temp;
        int j = 0;
        boolean found;
        CCBCLguiFunction tmpfunc = new CCBCLguiFunction("TEST");
        i = 0;
        
        /*
        ** we will go through our array of function names
        ** and internal functions and compare it to the
        ** function names and help text.  We will also try
        ** and determine parameters from the internal functions
        */
        while (i < func.size())
        {
            /*
            ** initialization
            */
            temp = (String[])func.elementAt(i);
            j = 0;
            quit = false;
            found = false;
           
            /*
            ** we will search the store Vector that contains the
            ** function name and help text to see if we can match
            ** the function name from our array
            */
            while (!quit)
            {
                tmpfunc = (CCBCLguiFunction)store.elementAt(j);
                
                if (tmpfunc.getFuncName().equals(temp[0]))
                {
                    found = true;
                    quit = true;
                }
                
                else if ((j+1) >= store.size())
                {
                    quit = true;
                }
                
                else
                {
                    ++j;
                }
            }
            
            /*
            ** if we did not find the function in Vector store
            ** than we have a function that exists but has no help
            ** text, so we need to add it to our store Vector
            */
            if(!found && temp[0] != null)
            {
                tmpfunc = new CCBCLguiFunction(temp[0]);
                store.add(tmpfunc);
                found = true;
            }  
            
            /*
            ** now that we have the function we will try and determine
            ** its parameters
            */
            if (found && !(temp[1].equals("")))
            {
                /*
                ** initialization
                */
                funcNameQuit = false;
                found = false;
                j = 0;
                bcount = 0;
                pcount = 0;
                
                /*
                ** first we will see if we can find the internal
                ** function in the ccbcl.pl
                */
                try{
                    index = file.indexOf(("sub " + temp[1]));
                }catch (Exception ue)
                { 
                    throw new Exception("cannot find \"sub " + temp[1] + "\"");
                }
                
                /*
                ** We found it!  now we need to get a substring
                ** starting at the new index so we can make an
                ** attempt to parse it and find any parameters
                */
                if (index >= 0)
                {
                    copy = new String(file.substring(index));
                    arr = copy.toCharArray();
                }
                else
                {
                    funcNameQuit = true;
                }
                
                /*
                ** run through the internal function until
                ** funcNameQuit = true.  We are looking for
                ** "@args;" or "@_;"
                */
                while (!funcNameQuit)
                {
                    switch (arr[j])
                    {
                        case '{':
                            /*
                            ** we want to keep a count of
                            ** our braces in the case of
                            ** nested braces
                            */
                            ++bcount;
                            break;
                            
                        case '}':
                            /*
                            ** if we have hit our last closing
                            ** brace quit the
                            ** loop, we did not find parameters
                            */
                            if (--bcount == 0)
                            {
                                funcNameQuit = true;
                            }
                            break;
                        
                        /*
                        ** These next few case statements
                        ** check for @args; or @_
                        */
                        case '@':
                            pcount = 1;
                            break;
                            
                        case '_':
                            if (pcount == 1)
                            {
                                pcount = 15;
                            }
                            else
                            {
                                pcount = 0;
                            }
                            break;
                            
                        case 'a':
                            if (pcount == 1)
                            {
                                ++pcount;
                            }
                            else
                            {
                                pcount = 0;
                            }
                            break;
                            
                        case 'r':
                            if (pcount == 2)
                            {
                                ++pcount;
                            }
                            else
                            {
                                pcount = 0;
                            }
                            break;
                            
                        case 'g':
                            if (pcount == 3)
                            {
                                ++pcount;
                            }
                            else
                            {
                                pcount = 0;
                            }
                            break;
                            
                        case 's':
                            if (pcount == 4)
                            {
                                ++pcount;
                            }
                            else
                            {
                                pcount = 0;
                            }
                            break;
                            
                        case ';':
                            /*
                            ** we have hit a semicolon, so we need to
                            ** check to see if one of the argument
                            ** conditions was met 5 or 15
                            ** if it has we can quit this loop and
                            ** set found to true and move on
                            */
                            if (pcount == 5 || pcount == 15)
                            {
                                funcNameQuit = true;
                                found = true;
                                if (pcount == 5)
                                {
                                    j -= 8;
                                }
                                else
                                {
                                    j -= 5;
                                }
                            }
                            
                            /*
                            ** else we start over again
                            */
                            else
                            {
                                pcount = 0;
                            }
                            break;
                        default:
                            break;
                    }
                    ++j;
                }
                
                /*
                ** if @args or @_ was found we will try to get
                ** the parameters
                */
                if (found && !(temp[1].equals("")))
                {
                    /*
                    ** initialization
                    */
                    hlpr = new String("");
                    funcNameQuit = false;
                    write = false;
                    
                    /*
                    ** we are going to work backwards now
                    ** to see if we can get the arguments
                    */
                    while(!funcNameQuit)
                    {
                        if (j <= 0)
                        {
                            funcNameQuit = true;
                        }
                        else
                        {
                           switch(arr[--j])
                           {
                               
                               case ')':
                                   /*
                                   ** we have found the beginning of
                                   ** the end of the argument list
                                   ** so we can set write to true
                                   */
                                   write = true;
                                   break;
                               case ' ':
                               case '=':
                               case '$':
                               case '\n':
                                   /*
                                   ** ignore these
                                   */
                                   break;
                               
                               case '@':
                                   /*
                                   ** this is here because currently
                                   ** there are some functions that
                                   ** just use the argument array
                                   ** directly and copy it into
                                   ** another array.  this prevents
                                   ** an unending loop
                                   */
                                   funcNameQuit = true;
                                   break;
                               
                               case ',':
                               case '(':
                                   /*
                                   ** if we get either of these we need to add
                                   ** what we have currently for our parameter
                                   ** name
                                   */
                                   tmpfunc.addParameter(hlpr);
                                   hlpr = new String("");
                                   
                                   /*
                                   ** if we get '(' we are at the beginning
                                   ** so we can quit
                                   */
                                   if(arr[j] == '(')
                                   {
                                       funcNameQuit = true;
                                   }
                                   break;
                               
                               default:
                                   /*
                                   ** if write, copy the character to hlpr
                                   */
                                   if(write)
                                   {
                                       hlpr = arr[j] + hlpr;
                                   }
                                   break;
                           }
                       }
                    }
                }
            }
            ++i;
        }

        /*
        ** return our Vector of functions we worked so hard for
        */
        return store;
    }
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.5  2003/04/17 22:09:19  HoltyB
** TBolt00000000:  Changes for new help text layout in CCBcl.pl.
**
** Revision 1.4  2003/01/23 16:48:13  NguyenT
** TBolt00006856: changed can not to cannot
**
** Revision 1.3  2002/06/25 22:04:52  HoltyB
** Updated to parse current ccbcl.pl
**
** Revision 1.2  2002/02/14 13:22:49  HoltyB
** Added function to change Exercisers being used
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/