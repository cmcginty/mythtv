#include <qsqldatabase.h>
#include <qfile.h>
#include <cmath>
#include <iostream>

using namespace std;

#include "metadata.h"

bool operator==(const Metadata& a, const Metadata& b)
{
    if (a.Filename() == b.Filename())
        return true;
    return false;
}

bool operator!=(const Metadata& a, const Metadata& b)
{
    if (a.Filename() != b.Filename())
        return true;
    return false;
}

void Metadata::setField(QString field, QString data)
{
    if (field == "title")
        title = data;
    else if (field == "director")
        director = data;
    else if (field == "plot")
        plot = data;
    else if (field == "rating")
        rating = data;
    else if (field == "year")
        year = data.toInt();
    else if (field == "userrating")
    {
        userrating = data.toFloat();
        if (isnan(userrating)) 
            userrating = 0.0;
        if (userrating < -10.0 || userrating >= 10.0)
            userrating = 0.0;
    }
    else if (field == "length")
        length = data.toInt();
    else if (field == "showlevel")
        showlevel = data.toInt();
    else if (field == "coverfile")
        coverfile = data;
    else if (field == "inetref")
        inetref = data;
    else if (field == "childid")
        childID = data.toUInt();
    else if (field == "browse")
    {
        bool browse_setting = false;
        bool ok;
        int browse_int_setting = data.toUInt(&ok);
        if(!ok)
        {
            cerr << "metadata.o: Problems setting the browse flag from this data: " << data << endl;
        }
        else
        {
            if(browse_int_setting)
            {
                browse_setting = true;
            }
        }
        browse = browse_setting;
    }
    else if (field == "playcommand")
    {
        playcommand = data;
    }
}

bool Metadata::Remove(QSqlDatabase *db)
{
    QFile videofile;
    videofile.setName(filename);
    bool isremoved = videofile.remove();
    if (isremoved)
    {
        QString thequery;
        thequery.sprintf("DELETE FROM videometadatagenre "
                        " WHERE idvideo = %d",id);
        QSqlQuery query(thequery,db);
        if (!query.isActive()){
                cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
        }
        thequery.sprintf("DELETE FROM videometadatacountry "
                        " WHERE idvideo = %d",id);
        query.exec(thequery);
        if (!query.isActive()){
                cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
        }
        thequery.sprintf("DELETE FROM videometadata "
                        " WHERE intid = %d",id);
        query.exec(thequery);
        if (!query.isActive()){
                cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
        }
        thequery = QString("DELETE FROM filemarkup WHERE filename = '%1'")
                           .arg(filename);
        query.exec(thequery);
        if (!query.isActive()){
                cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
        }

    }
    else
    {
            cerr << "impossible de supprimmer le fichier" << endl;
    }
    return isremoved;
}

void Metadata::fillCategory(QSqlDatabase *db)
{
    QString thequery;
    thequery.sprintf("SELECT videocategory.category"
                    " FROM videometadata INNER JOIN videocategory"
                    " ON videometadata.category = videocategory.intid"
                    " WHERE videometadata.intid = %d",id);
    QSqlQuery query(thequery,db);
    if (query.isActive())
    {
        if (query.numRowsAffected()>0)
        {
                query.next();
                category = query.value(0).toString();
        }
    }
    else
    {
            cerr << "metadata.o : SELECT Failed : " << thequery << endl;
    }

}
void Metadata::fillGenres(QSqlDatabase *db)
{
    QString thequery;
    thequery.sprintf("SELECT genre"
                    " FROM videometadatagenre INNER JOIN videogenre"
                    " ON videometadatagenre.idgenre = videogenre.intid"
                    " WHERE idvideo=%d",id);
    QSqlQuery query(thequery,db);
    genres.clear();
    if (query.isActive() && query.numRowsAffected()>1)
    {
        while(query.next())
        {
            genres.append(query.value(0).toString());
        }
    }
}

void Metadata::fillCountries(QSqlDatabase *db)
{
    QString thequery;
    thequery.sprintf("SELECT country" 
                    " FROM videometadatacountry INNER JOIN videocountry"
                    " ON videometadatacountry.idcountry = videocountry.intid"
                    " WHERE idvideo=%d",id);
    QSqlQuery query(thequery,db);
    countries.clear();
    if (query.isActive() && query.numRowsAffected() > 1)
    {
        while(query.next())
        {
            countries.append(query.value(0).toString());
        }
    }

}

bool Metadata::fillData(QSqlDatabase *db)
{
    if (title == "")
        return false;

    QString thequery = "SELECT title,director,plot,rating,year,userrating,"
                       "length,filename,showlevel,intid,coverfile,inetref,"
                       "childid, browse, playcommand FROM videometadata WHERE title=\"" + 
                        title + "\"";

    if (director != "")
        thequery += " AND director=\"" + director + "\"";
    if (plot != "")
        thequery += " AND plot=\"" + plot + "\"";

    thequery += ";";

    QSqlQuery query = db->exec(thequery);

    if (query.isActive() && query.numRowsAffected() > 0)
    {
        query.next();

        title = QString::fromUtf8(query.value(0).toString());
        director = QString::fromUtf8(query.value(1).toString());
        plot = QString::fromUtf8(query.value(2).toString());
        rating = QString::fromUtf8(query.value(3).toString());
        year = query.value(4).toInt();
        userrating = (float)query.value(5).toDouble();
        if (isnan(userrating)) 
            userrating = 0.0;
        if (userrating < -10.0 || userrating >= 10.0)
            userrating = 0.0;
        length = query.value(6).toInt();
        filename = QString::fromUtf8(query.value(7).toString());
        showlevel = query.value(8).toInt();
        id = query.value(9).toUInt();
        coverfile = QString::fromUtf8(query.value(10).toString());
        inetref = QString::fromUtf8(query.value(11).toString());
        childID = query.value(12).toUInt();
        browse = query.value(13).toBool();
        playcommand = query.value(14).toString();
    return true;
    }
    else
    {
        return false;
    }
}

bool Metadata::fillDataFromID(QSqlDatabase *db)
{       
    if (id == 0)
        return false; 
        
    
    QString thequery;
    thequery = QString("SELECT title,director,plot,rating,year,userrating,"
                       "length,filename,showlevel,coverfile,inetref,childid,"
                       "browse,playcommand, videocategory.category "
                       " FROM videometadata LEFT JOIN videocategory"
                       " ON videometadata.category = videocategory.intid"
                       "  WHERE videometadata.intid=%1;")
                       .arg(id);
    QSqlQuery query = db->exec(thequery);

    if (query.isActive() && query.numRowsAffected() > 0)
    {
        query.next();

        title = QString::fromUtf8(query.value(0).toString());
        director = QString::fromUtf8(query.value(1).toString());
        plot = QString::fromUtf8(query.value(2).toString());
        rating = query.value(3).toString();
        year = query.value(4).toInt();
        userrating = (float)query.value(5).toDouble();
        if (isnan(userrating)) 
            userrating = 0.0;
        if (userrating < -10.0 || userrating >= 10.0)
            userrating = 0.0;
        length = query.value(6).toInt();
        filename = QString::fromUtf8(query.value(7).toString());
        showlevel = query.value(8).toInt();
        coverfile = QString::fromUtf8(query.value(9).toString());
        inetref = QString::fromUtf8(query.value(10).toString());
        childID = query.value(11).toUInt();
        browse = query.value(12).toBool();
        playcommand = query.value(13).toString();
        category = query.value(14).toString();

    // Genres
        fillGenres(db);
    
        //Countries
        fillCountries(db);

    return true;
    }
    else
    {
        cerr << "metadata.o : SELECT by id failed : " << thequery << endl;
    return false;
    }
}

bool Metadata::fillDataFromFilename(QSqlDatabase *db)
{       
    if (filename == "")
        return false; 
        
    
    QString thequery;
    thequery = QString("SELECT videometadata.intid,title,director,plot"
               ",rating,year,userrating,length,showlevel,coverfile"
               ",inetref,childid,browse,playcommand"
               ", videocategory.category "
                       " FROM videometadata LEFT JOIN videocategory"
                       " ON videometadata.category = videocategory.intid"
                       "  WHERE videometadata.filename='%1';")
                       .arg(filename.utf8());
    QSqlQuery query = db->exec(thequery);
    if (query.isActive() && query.numRowsAffected() > 0)
    {
        query.next();

    id = query.value(0).toInt();
        title = QString::fromUtf8(query.value(1).toString());
        director = QString::fromUtf8(query.value(2).toString());
        plot = QString::fromUtf8(query.value(3).toString());
        rating = query.value(4).toString();
        year = query.value(5).toInt();
        userrating = (float)query.value(6).toDouble();
        if (isnan(userrating)) 
            userrating = 0.0;
        if (userrating < -10.0 || userrating >= 10.0)
            userrating = 0.0;
        length = query.value(7).toInt();
        showlevel = query.value(8).toInt();
        coverfile = QString::fromUtf8(query.value(9).toString());
        inetref = QString::fromUtf8(query.value(10).toString());
        childID = query.value(11).toUInt();
        browse = query.value(12).toBool();
        playcommand = query.value(13).toString();
        category = query.value(14).toString();

    // Genres
        fillGenres(db);
    
        //Countries
        fillCountries(db);
    return true;
    }
    else
    {
        cerr << "metadata.o : SELECT by filename failed : " << thequery << endl;
    return false;
    }
}

void Metadata::dumpToDatabase(QSqlDatabase *db)
{
    if (title == "")
        title = filename;
    if (director == "")
        director = QObject::tr("Unknown");
    if (plot == "")
        plot = QObject::tr("None");
    if (rating == "")
        rating = QObject::tr("Unknown Rating");
    if (coverfile == "")
        coverfile = QObject::tr("No Cover");
    if (inetref == "")
        inetref = "00000000";
    
    if(gContext->GetNumSetting("VideoNewBrowsable", 1))
    {
        browse = true;
    }
    else
    {
        browse = false;
    }

    title.replace(QRegExp("\""), QString("\\\""));
    director.replace(QRegExp("\""), QString("\\\""));
    plot.replace(QRegExp("\""), QString("\\\""));
    rating.replace(QRegExp("\""), QString("\\\""));

    QString sqlfilename = filename;
    sqlfilename.replace(QRegExp("\""), QString("\\\""));

    QString sqlcoverfile = coverfile;
    sqlcoverfile.replace(QRegExp("\""), QString("\\\""));

    QString thequery;
    thequery.sprintf("INSERT INTO videometadata (title,director,plot,"
                     "rating,year,userrating,length,filename,showlevel,"
                     "coverfile,inetref,browse) VALUES "
                     "(\"%s\",\"%s\",\"%s\",\"%s\",%d,%f,%d,\"%s\",%d,\"%s\","
                     "\"%s\", %d);",
                     title.utf8().data(), director.utf8().data(),
                     plot.utf8().data(), rating.utf8().data(), year,
                     userrating, length, sqlfilename.utf8().data(), showlevel,
                     sqlcoverfile.utf8().data(), inetref.utf8().data(),browse);
    
    
    
    QSqlQuery a_query(thequery, db);

    if (!a_query.isActive() || a_query.numRowsAffected() < 1)
    {
        cerr << "metadata.o: The following metadata insert failed :" << thequery << endl;
        return;
    }

    // Must make sure we have 'id' filled before we call updateGenres or updateCountries
    thequery = "SELECT LAST_INSERT_ID();";
    a_query.exec(thequery);

    if (!a_query.isActive() || a_query.numRowsAffected() < 1)
    {
        cerr << "metadata.o: The following metadata id retreval failed :" << thequery << endl;
        return;      
    }      

    a_query.next();
    id = a_query.value(0).toUInt();

    if (0 == id)
    {
        cerr << "metadata.o: The id of the last inserted row to videometadata seems to be 0. This is odd." << endl;
        return;
    }

    updateGenres(db);
    updateCountries(db);
}

void Metadata::guessTitle()
{
    title = filename.right(filename.length() - filename.findRev("/") - 1);
    title.replace(QRegExp("_"), " ");
    title.replace(QRegExp("%20"), " ");
    title = title.left(title.findRev("."));
    title.replace(QRegExp("\\."), " ");
    
    eatBraces("[", "]");
    eatBraces("(", ")");
    eatBraces("{", "}");
    
    title = title.stripWhiteSpace();
}

void Metadata::eatBraces(const QString &left_brace, const QString &right_brace)
{
    bool keep_checking = true;
    
    while(keep_checking)
    {
        int left_position = title.find(left_brace);
        int right_position = title.find(right_brace);
        if(left_position == -1 || right_position == -1)
        {
            //
            //  No matching sets of these braces left.
            //

            keep_checking = false;
        }
        else
        {
            if(left_position < right_position)
            {
                //
                //  We have a matching set like:  (  foo  )
                //

                title = title.left(left_position) + 
                        title.right(title.length() - right_position - 1);
            }
            else if (left_position > right_position)
            {
                //
                //  We have a matching set like:  )  foo  (
                //

                title = title.left(right_position) + 
                        title.right(title.length() - left_position - 1);
            }
        }
    }
}

void Metadata::updateDatabase(QSqlDatabase *db)
{
    if (title == "")
        guessTitle();
    if (director == "")
        director = QObject::tr("Unknown");
    if (plot == "")
        plot = QObject::tr("None");
    if (rating == "")
        rating = QObject::tr("Unknown Rating");
    if (coverfile == "")
        coverfile = QObject::tr("No Cover");
    if (inetref == "")
        inetref = "00000000";

    title.replace(QRegExp("\""), QString("\\\""));
    director.replace(QRegExp("\""), QString("\\\""));
    plot.replace(QRegExp("\""), QString("\\\""));
    rating.replace(QRegExp("\""), QString("\\\""));
    playcommand.replace(QRegExp("\""), QString("\\\""));
    QString sqlfilename = filename;
    sqlfilename.replace(QRegExp("\""), QString("\\\""));

    QString sqlcoverfile = coverfile;
    sqlcoverfile.replace(QRegExp("\""), QString("\\\""));

    int idCategory = getIdCategory(db);

    QString thequery;
    thequery.sprintf("UPDATE videometadata SET title=\"%s\",director=\"%s\","
                     "plot=\"%s\",rating=\"%s\",year=%d,userrating=%f,"
                     "length=%d,filename=\"%s\",showlevel=%d,coverfile=\"%s\","
                     "inetref=\"%s\",browse=%d,playcommand=\"%s\",childid=%d,"
                     "category=%d"
                     " WHERE intid=%d",
                     title.utf8().data(), director.utf8().data(),
                     plot.utf8().data(), rating.utf8().data(), year,
                     userrating, length, sqlfilename.utf8().data(), showlevel,
                     sqlcoverfile.utf8().data(), inetref.utf8().data(), browse,
                     playcommand.utf8().data(), childID, idCategory, id);

    QSqlQuery a_query(thequery, db);
    if(!a_query.isActive())
    {
        
        cerr << "metadata.o: The following metadata update failed: " << thequery << endl;
    }
    updateGenres(db);
    updateCountries(db);
}

int Metadata::getIdCategory(QSqlDatabase *db)
{
    int idcategory = 0;
    if (category != "")
    {
        QString thequery;
        thequery.sprintf("SELECT intid FROM videocategory"
                        " WHERE category like \"%s\"",category.utf8().data());
        QSqlQuery a_query(thequery,db);
        if (a_query.isActive() && a_query.numRowsAffected()>0)
        {
            a_query.next();
            idcategory = a_query.value(0).toInt();
        }
        else
        {
            thequery.sprintf("INSERT INTO videocategory (category)"
                             " VALUES (\"%s\");", category.utf8().data());
            a_query.exec(thequery);
            if (a_query.isActive() && a_query.numRowsAffected()>0)
            {
                thequery.sprintf("SELECT intid"
                                " FROM videocategory"
                                " WHERE category like \"%s\"",
                                category.utf8().data());
                a_query.exec(thequery);
                if (a_query.isActive() && 
                        a_query.numRowsAffected()>0)
                {
                        a_query.next();
                        idcategory = a_query.value(0).toInt();
                }
                else
                    cerr << "metadata.o : SELECT Failed : " << thequery << endl;
            }
        }
    }
    return idcategory;
    
}

void Metadata::setIdCategory(QSqlDatabase *db, int id)
{
    if (id==0)
            category = "";
    else 
    {
        QString thequery;
        thequery.sprintf("SELECT category FROM videocategory"
                        " WHERE intid = \"%d\"",id);
        QSqlQuery a_query(thequery,db);
        if (a_query.isActive() && a_query.numRowsAffected()>0)
        {
            a_query.next();
            category = a_query.value(0).toString();
        }
    }
    
}

void Metadata::updateGenres(QSqlDatabase *db)
{
    QString thequery;
    //remove genres  for this video
    thequery.sprintf("DELETE FROM videometadatagenre where idvideo=%d",id);
    QSqlQuery a_query(thequery,db);
    if (!a_query.isActive())
    {
        cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
    }
    QStringList::Iterator genre;
    for (genre = genres.begin() ; genre != genres.end() ; ++genre)
    {
        // Search id of genre
        thequery.sprintf("SELECT intid FROM videogenre where genre like \"%s\";",(*genre).utf8().data());
        a_query.exec(thequery);
        int idgenre=0;
        if (a_query.isActive())
        {
            if (a_query.numRowsAffected()>0)
            {
                a_query.next();
                idgenre = a_query.value(0).toInt();
            }
            else
            {
                //We must add a new genre
                thequery.sprintf("INSERT INTO videogenre (genre) VALUES (\"%s\");",
                                 (*genre).utf8().data());
                a_query.exec(thequery);
                if (a_query.isActive() && a_query.numRowsAffected() > 0)
                {
                    //search the new idgenre
                    thequery = "SELECT LAST_INSERT_ID();";
                    a_query.exec(thequery);
                    if (a_query.isActive())
                    {
                        if (a_query.numRowsAffected()>0)
                        {
                            a_query.next();
                            idgenre=a_query.value(0).toInt();
                        }
                    }
                    else
                    {
                        cerr << "metadata.o : The following id retreval failed : " << thequery << endl;
                    }
                }
                else
                {
                    cerr << "metadata.o : The Following insert failed" << thequery << endl;
                }
            }
        }
        else
        {
        cerr << "metadata.o : The Following search failed : "<< thequery << endl;
        }
        
        if (idgenre>0)
        {
            // add current genre for this video
            thequery.sprintf("INSERT INTO videometadatagenre (idvideo, idgenre) "
                             "VALUES (%d,\"%d\")",id, idgenre);
            a_query.exec(thequery);
            if (!a_query.isActive())
            {
            cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
            }
        }
    }
}

void Metadata::updateCountries(QSqlDatabase *db)
{
    QString thequery;
    //remove countries for this video
    thequery.sprintf("DELETE FROM videometadatacountry where idvideo=%d",id);
    QSqlQuery a_query(thequery,db);
    if (!a_query.isActive())
    {
        cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
    }
    QStringList::Iterator country;
    for (country = countries.begin() ; country != countries.end() ; ++country)
    {
        // Search id of country
        thequery.sprintf("SELECT intid FROM videocountry where country "
                         "like \"%s\";",(*country).utf8().data());
        a_query.exec(thequery);
        int idcountry=0;
        if (a_query.isActive())
        {
            if (a_query.numRowsAffected()>0)
            {
                a_query.next();
                idcountry = a_query.value(0).toInt();
            }
            else
            {
                //We must add a new country
                thequery.sprintf("INSERT INTO videocountry (country) "
                                 "VALUES (\"%s\");",(*country).utf8().data());
                a_query.exec(thequery);
                if (a_query.isActive() && a_query.numRowsAffected() > 0)
                {
                    //search the new idcountry
                    thequery = "SELECT LAST_INSERT_ID();";
                    a_query.exec(thequery);
                    if (a_query.isActive())
                    {
                        if (a_query.numRowsAffected()>0)
                        {
                                a_query.next();
                                idcountry=a_query.value(0).toInt();
                        }
                    }
                    else
                    {
                        cerr << "metadata.o : The following id retreval failed : " << thequery << endl;
                    }
                }
                else
                {
                    cerr << "metadata.o : The Following insert failed" << thequery << endl;
                }
            }
        }
        else
        {
            cerr << "metadata.o : The Following search failed : "<< thequery << endl;
        }
        
        if (idcountry>0)
        {
            // add current country for this video
            thequery.sprintf("INSERT INTO videometadatacountry (idvideo, idcountry) "
                             "VALUES (%d,\"%d\")",id, idcountry);
            a_query.exec(thequery);
            if (!a_query.isActive())
            {
            cerr << "metadata.o: The following metadata update failed :" << thequery << endl;
            }
        }
    }
}


QImage* Metadata::getCoverImage()
{
    if (!coverImage && (CoverFile() != QObject::tr("No Cover")) && (CoverFile() != QObject::tr("None")))
    {
        coverImage = new QImage();
        if (!coverImage->load(coverfile))
        {
            delete coverImage;
            coverImage = NULL;
        }
    }
        
    return coverImage;
}


QPixmap* Metadata::getCoverPixmap()
{
    if (coverPixmap)
        return coverPixmap;
    
    if (coverfile)
    {
        coverPixmap = new QPixmap();
        coverPixmap->load(coverfile);
    }
    return coverPixmap;
}
